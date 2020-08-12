# Copyright (c) Facebook, Inc. and its affiliates
from typing import Dict, List, Optional, Tuple

import torch
import torch.distributions as dist
from beanmachine.ppl.inference.proposer.single_site_ancestral_proposer import (
    SingleSiteAncestralProposer,
)
from beanmachine.ppl.model.utils import RVIdentifier
from beanmachine.ppl.world import ProposalDistribution, Variable, World
from beanmachine.ppl.world.variable import TransformType
from torch import Tensor, tensor


class SingleSiteRandomWalkProposer(SingleSiteAncestralProposer):
    """
    Single-Site Random Walk Metropolis Hastings Implementations

    For random variables with continuous support distributions, returns a
    sample with a perturbation added to the current variable.
    For the rest of the random variables, it returns ancestral metropolis hastings
    proposal.
    """

    def __init__(
        self,
        step_size: float,
        transform_type: TransformType = TransformType.NONE,
        transforms: Optional[List] = None,
    ):
        """
        Initialize object.

        :param step_size: Standard Deviation of the noise used for Diff proposals.
        """

        self.step_size = step_size
        super().__init__(transform_type, transforms)
        # Key is bool, indicates: is r.v. multidimensional?
        self.target_acc_rate = {False: tensor(0.44), True: tensor(0.234)}

    def do_adaptation(
        self,
        node: RVIdentifier,
        world: World,
        acceptance_probability: Tensor,
        iteration_number: int,
        num_adaptive_samples: int,
        is_accepted: bool,
    ) -> None:
        """
        Adapted from Garthwaite, Fan, Sisson, 2016, to be done online.

        :param node: the node for which we have already proposed a new value for.
        :param node_var:
        :param node_acceptance_results: the boolean values of acceptances for
         values collected so far within _infer().
        :param iteration_number: The current iteration of inference
        :param num_adaptive_samples: The number of inference iterations for adaptation.
        :param is_accepted: bool representing whether the new value was accepted.
        :returns: Nothing.
        """

        node_var = world.get_node_in_world_raise_error(node, False)
        if node_var.value.shape[0] == 1:
            target_acc_rate = self.target_acc_rate[False]
            c = torch.reciprocal(target_acc_rate)
        else:
            target_acc_rate = self.target_acc_rate[True]
            c = torch.reciprocal(1.0 - target_acc_rate)

        new_step_size = self.step_size * torch.exp(
            (acceptance_probability - target_acc_rate) * c / (iteration_number + 1.0)
        )

        self.step_size = new_step_size.item()

    def get_proposal_distribution(
        self,
        node: RVIdentifier,
        node_var: Variable,
        world: World,
        auxiliary_variables: Dict,
    ) -> Tuple[ProposalDistribution, Dict]:
        """
        Returns the proposal distribution of the node.

        :param node: the node for which we're proposing a new value for
        :param node_var: the Variable of the node
        :param world: the world in which we're proposing a new value for node
        :param auxiliary_variables: additional auxiliary variables that may be
        required to find a proposal distribution
        :returns: the tuple of proposal distribution of the node and arguments
        that was used or needs to be used to find the proposal distribution
        """
        node_distribution = node_var.distribution

        # for now, assume all transforms will transform distributions into the realspace
        if world.get_transforms_for_node(
            node
        ).transform_type != TransformType.NONE or isinstance(
            # pyre-fixme
            node_distribution.support,
            dist.constraints._Real,
        ):
            return (
                ProposalDistribution(
                    proposal_distribution=dist.Normal(
                        node_var.transformed_value,
                        torch.ones(node_var.transformed_value.shape) * self.step_size,
                    ),
                    requires_transform=True,
                    requires_reshape=False,
                    arguments={},
                ),
                {},
            )
        elif isinstance(
            node_distribution.support, dist.constraints._GreaterThan
        ) or isinstance(node_distribution.support, dist.constraints._GreaterThan):
            lower_bound = node_distribution.support.lower_bound
            proposal_distribution = self.gamma_distbn_from_moments(
                node_var.value - lower_bound, self.step_size ** 2
            )
            transform = dist.AffineTransform(loc=lower_bound, scale=1.0)
            transformed_proposal = dist.TransformedDistribution(
                proposal_distribution, transform
            )

            return (
                ProposalDistribution(
                    proposal_distribution=transformed_proposal,
                    requires_transform=False,
                    requires_reshape=False,
                    arguments={},
                ),
                {},
            )
        elif isinstance(node_distribution.support, dist.constraints._Interval):
            lower_bound = node_distribution.support.lower_bound
            width = node_distribution.support.upper_bound - lower_bound
            # Compute first and second moments of the perturbation distribution
            # by rescaling the support
            mu = (node_var.value - lower_bound) / width
            sigma = torch.ones(node_var.value.shape) * self.step_size / width
            proposal_distribution = self.beta_distbn_from_moments(mu, sigma)
            transform = dist.AffineTransform(loc=lower_bound, scale=width)
            transformed_proposal = dist.TransformedDistribution(
                proposal_distribution, transform
            )

            return (
                ProposalDistribution(
                    proposal_distribution=transformed_proposal,
                    requires_transform=False,
                    requires_reshape=False,
                    arguments={},
                ),
                {},
            )
        elif isinstance(node_distribution.support, dist.constraints._Simplex):
            proposal_distribution = self.dirichlet_distbn_from_moments(
                node_var.value, self.step_size
            )
            return (
                ProposalDistribution(
                    proposal_distribution=proposal_distribution,
                    requires_transform=False,
                    requires_reshape=False,
                    arguments={},
                ),
                {},
            )
        else:
            return super().get_proposal_distribution(
                node, node_var, world, auxiliary_variables
            )

    def gamma_distbn_from_moments(self, expectation, sigma):
        """
        Returns a Gamma distribution.

        :param expectation: expectation value
        :param sigma: sigma value
        :returns: returns the Beta distribution given mu and sigma.
        """
        beta = expectation / (sigma ** 2)
        alpha = expectation * beta
        distribution = dist.Gamma(concentration=alpha, rate=beta)
        return distribution

    def beta_distbn_from_moments(self, mu, sigma):
        """
        Returns a Beta distribution.

        :param mu: mu value
        :param sigma: sigma value
        :returns: returns the Beta distribution given mu and sigma.
        """
        mu = torch.clamp(mu, 1e-3, 1 - 1e-3)
        """
        https://stats.stackexchange.com/questions/12232/calculating-the-
        parameters-of-a-beta-distribution-using-the-mean-and-variance
        """
        alpha = ((1.0 - mu) / (sigma ** 2) - (1.0 / mu)) * (mu ** 2)
        beta = alpha * (1.0 - mu) / mu
        distribution = dist.Beta(concentration1=alpha, concentration0=beta)
        return distribution

    def dirichlet_distbn_from_moments(self, mu, sigma):
        """
        Returns a Dirichlet distribution. The variances of a Dirichlet
        distribution are inversely proportional to the norm of the concentration
        vector. However, variance is only set as a scalar, not as a vector.
        So the individual variances of the Dirichlet are not tuned, only the
        magnitude of the entire vector.

        :param mu: mu value
        :param sigma: sigma value
        :returns: returns the Dirichlet distribution given mu and sigma.
        """
        alpha = mu / (torch.norm(mu) * sigma ** 2)
        return dist.Dirichlet(concentration=alpha)