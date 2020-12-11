"""Tests for hypothesis_testing.py"""
import unittest

from beanmachine.ppl.testlib.hypothesis_testing import (
    inverse_normal_cdf,
    mean_equality_hypothesis_confidence_interval,
    mean_equality_hypothesis_test,
)
from torch import tensor


class HypothesisTestingTest(unittest.TestCase):
    def test_hypothesis_test_inverse_normal_cdf(self) -> None:
        """Minimal test for inverse normal CDF used to calculate z values"""

        # Check that the median has the probability we expect
        median = inverse_normal_cdf(0.5)
        self.assertEqual(
            median, 0.0, msg="Unexpected value for median of normal distribution"
        )

        # Record and check the values we get for z_0.01
        expected_z_one_percent = -2.3263478740408408
        observed_z_one_percent = inverse_normal_cdf(0.01)
        self.assertEqual(
            observed_z_one_percent,
            expected_z_one_percent,
            msg="Expected value for z_0.01",
        )

        # Record and check the values we get for z_0.99
        expected_z_99_percent = 2.3263478740408408
        observed_z_99_percent = inverse_normal_cdf(1 - 0.01)
        self.assertEqual(
            observed_z_99_percent,
            expected_z_99_percent,
            msg="Expected value for z_0.99",
        )

        # Record and check the values we get for z_0.005
        expected_z_half_percent = -2.575829303548901
        observed_z_half_percent = inverse_normal_cdf(0.005)
        self.assertEqual(
            observed_z_half_percent,
            expected_z_half_percent,
            msg="Expected value for z_0.005",
        )

        # This example shows why 1-p can be problematic
        # Compare this value to -expected_z_half_percent
        expected_z_995_thousandths = 2.5758293035489004
        observed_z_995_thousandths = inverse_normal_cdf(0.995)
        self.assertTrue(
            not (expected_z_995_thousandths == -expected_z_half_percent),
            msg="Numerical z_p is usually not exactly -z_(1-p)",
        )
        self.assertEqual(
            observed_z_995_thousandths,
            expected_z_995_thousandths,
            msg="Expected value for z_0.005",
        )

    def test_hypothesis_test_main(self) -> None:
        """Minimal test for mean equality hypothesis test"""
        sample_mean = tensor(10)
        true_mean = tensor(0)
        true_std = tensor(1)
        sample_size = tensor(1)
        p_value = 0.01
        observed_result = mean_equality_hypothesis_test(
            sample_mean, true_mean, true_std, sample_size, p_value
        )
        self.assertFalse(observed_result, msg="Mean is not within confidence interval")

        sample_mean = tensor(0)
        true_mean = tensor(0)
        true_std = tensor(1)
        sample_size = tensor(1)
        p_value = 0.01
        observed_result = mean_equality_hypothesis_test(
            sample_mean, true_mean, true_std, sample_size, p_value
        )
        self.assertTrue(observed_result, msg="Mean is not within confidence interval")

        # This test case is at the edge of acceptable.
        # It should pass because of the = in <= in the
        # mean_equality_hypothesis_test method
        expected_z_995_thousandths = 2.5758293035489004
        sample_mean = tensor(expected_z_995_thousandths)
        true_mean = tensor(0)
        true_std = tensor(1)
        sample_size = tensor(1)
        p_value = 0.01
        observed_result = mean_equality_hypothesis_test(
            sample_mean, true_mean, true_std, sample_size, p_value
        )
        self.assertTrue(observed_result, msg="Mean is not within confidence interval")

        # The following two tests are pushing the edge case around what
        # should be acceptable to the test. It is strange that the one
        # slighly larger than the alpha value does not fail.
        # TODO: Investigate and explain why this passes when it should be
        # just outside the acceptable boundary.
        expected_z_995_thousandths = 2.5758293035489004
        sample_mean = tensor(expected_z_995_thousandths * 1.00000001)
        true_mean = tensor(0)
        true_std = tensor(1)
        sample_size = tensor(1)
        p_value = 0.01
        observed_result = mean_equality_hypothesis_test(
            sample_mean, true_mean, true_std, sample_size, p_value
        )
        self.assertTrue(observed_result, msg="Mean is not within confidence interval")

        # This one, with bigger multiplierf, finally returns False
        expected_z_995_thousandths = 2.5758293035489004
        sample_mean = tensor(expected_z_995_thousandths * 1.0000001)
        true_mean = tensor(0)
        true_std = tensor(1)
        sample_size = tensor(1)
        p_value = 0.01
        observed_result = mean_equality_hypothesis_test(
            sample_mean, true_mean, true_std, sample_size, p_value
        )
        self.assertFalse(observed_result, msg="Mean is not within confidence interval")

    def test_confidence_interval_mean(self) -> None:
        """Minimal test for mean confidence interval"""
        sample_mean = tensor(2)
        true_mean = tensor(0)
        true_std = tensor(1)
        sample_size = tensor(1)
        p_value = 0.05
        lower_bound, upper_bound = mean_equality_hypothesis_confidence_interval(
            true_mean, true_std, sample_size, p_value
        )
        observed_result = lower_bound <= sample_mean <= upper_bound
        self.assertFalse(observed_result, msg="Mean is not within confidence interval")

        sample_mean = tensor(1.95)
        true_mean = tensor(0)
        true_std = tensor(1)
        sample_size = tensor(1)
        p_value = 0.05
        lower_bound, upper_bound = mean_equality_hypothesis_confidence_interval(
            true_mean, true_std, sample_size, p_value
        )
        observed_result = lower_bound <= sample_mean <= upper_bound
        self.assertTrue(observed_result, msg="Mean is not within confidence interval")