"use strict";(self.webpackChunkwebsite=self.webpackChunkwebsite||[]).push([[4775],{3905:function(e,t,n){n.r(t),n.d(t,{MDXContext:function(){return s},MDXProvider:function(){return m},mdx:function(){return b},useMDXComponents:function(){return p},withMDXComponents:function(){return u}});var r=n(67294);function o(e,t,n){return t in e?Object.defineProperty(e,t,{value:n,enumerable:!0,configurable:!0,writable:!0}):e[t]=n,e}function i(){return i=Object.assign||function(e){for(var t=1;t<arguments.length;t++){var n=arguments[t];for(var r in n)Object.prototype.hasOwnProperty.call(n,r)&&(e[r]=n[r])}return e},i.apply(this,arguments)}function a(e,t){var n=Object.keys(e);if(Object.getOwnPropertySymbols){var r=Object.getOwnPropertySymbols(e);t&&(r=r.filter((function(t){return Object.getOwnPropertyDescriptor(e,t).enumerable}))),n.push.apply(n,r)}return n}function l(e){for(var t=1;t<arguments.length;t++){var n=null!=arguments[t]?arguments[t]:{};t%2?a(Object(n),!0).forEach((function(t){o(e,t,n[t])})):Object.getOwnPropertyDescriptors?Object.defineProperties(e,Object.getOwnPropertyDescriptors(n)):a(Object(n)).forEach((function(t){Object.defineProperty(e,t,Object.getOwnPropertyDescriptor(n,t))}))}return e}function c(e,t){if(null==e)return{};var n,r,o=function(e,t){if(null==e)return{};var n,r,o={},i=Object.keys(e);for(r=0;r<i.length;r++)n=i[r],t.indexOf(n)>=0||(o[n]=e[n]);return o}(e,t);if(Object.getOwnPropertySymbols){var i=Object.getOwnPropertySymbols(e);for(r=0;r<i.length;r++)n=i[r],t.indexOf(n)>=0||Object.prototype.propertyIsEnumerable.call(e,n)&&(o[n]=e[n])}return o}var s=r.createContext({}),u=function(e){return function(t){var n=p(t.components);return r.createElement(e,i({},t,{components:n}))}},p=function(e){var t=r.useContext(s),n=t;return e&&(n="function"==typeof e?e(t):l(l({},t),e)),n},m=function(e){var t=p(e.components);return r.createElement(s.Provider,{value:t},e.children)},d={inlineCode:"code",wrapper:function(e){var t=e.children;return r.createElement(r.Fragment,{},t)}},f=r.forwardRef((function(e,t){var n=e.components,o=e.mdxType,i=e.originalType,a=e.parentName,s=c(e,["components","mdxType","originalType","parentName"]),u=p(n),m=o,f=u["".concat(a,".").concat(m)]||u[m]||d[m]||i;return n?r.createElement(f,l(l({ref:t},s),{},{components:n})):r.createElement(f,l({ref:t},s))}));function b(e,t){var n=arguments,o=t&&t.mdxType;if("string"==typeof e||o){var i=n.length,a=new Array(i);a[0]=f;var l={};for(var c in t)hasOwnProperty.call(t,c)&&(l[c]=t[c]);l.originalType=e,l.mdxType="string"==typeof e?e:o,a[1]=l;for(var s=2;s<i;s++)a[s]=n[s];return r.createElement.apply(null,a)}return r.createElement.apply(null,n)}f.displayName="MDXCreateElement"},76002:function(e,t,n){n.r(t),n.d(t,{frontMatter:function(){return l},contentTitle:function(){return c},metadata:function(){return s},toc:function(){return u},default:function(){return m}});var r=n(87462),o=n(63366),i=(n(67294),n(3905)),a=["components"],l={id:"installation",title:"Installation",sidebar_label:"Installation"},c=void 0,s={unversionedId:"overview/installation/installation",id:"overview/installation/installation",title:"Installation",description:"Did You Check Out Colab?",source:"@site/../docs/overview/installation/installation.md",sourceDirName:"overview/installation",slug:"/overview/installation/",permalink:"/docs/overview/installation/",editUrl:"https://github.com/facebookresearch/beanmachine/edit/main/website/../docs/overview/installation/installation.md",tags:[],version:"current",frontMatter:{id:"installation",title:"Installation",sidebar_label:"Installation"},sidebar:"someSidebar",previous:{title:"Analysis",permalink:"/docs/overview/analysis/"},next:{title:"Overview",permalink:"/docs/inference"}},u=[{value:"Did You Check Out Colab?",id:"did-you-check-out-colab",children:[],level:2},{value:"Requirements",id:"requirements",children:[],level:2},{value:"Latest Release",id:"latest-release",children:[],level:2},{value:"Installing From Source",id:"installing-from-source",children:[],level:2}],p={toc:u};function m(e){var t=e.components,n=(0,o.Z)(e,a);return(0,i.mdx)("wrapper",(0,r.Z)({},p,n,{components:t,mdxType:"MDXLayout"}),(0,i.mdx)("h2",{id:"did-you-check-out-colab"},"Did You Check Out Colab?"),(0,i.mdx)("p",null,"The Google Colaboratory web service (Colab) is probably the quickest way to run Bean Machine. For example, ",(0,i.mdx)("a",{parentName:"p",href:"https://colab.research.google.com/github/facebookresearch/beanmachine/blob/main/tutorials/Coin_flipping.ipynb"},"here is what our Coin Flipping tutorial looks like on Colab"),". Similar links can be found for each of our tutorials in the Tutorials section."),(0,i.mdx)("h2",{id:"requirements"},"Requirements"),(0,i.mdx)("p",null,"Python 3.7-3.8 and PyTorch 1.10."),(0,i.mdx)("p",null,"Note: Some features are not yet supported on Python 3.9+"),(0,i.mdx)("h2",{id:"latest-release"},"Latest Release"),(0,i.mdx)("p",null,"Using ",(0,i.mdx)("inlineCode",{parentName:"p"},"pip")," you can get the latest release with the following command:"),(0,i.mdx)("pre",null,(0,i.mdx)("code",{parentName:"pre"},"pip install beanmachine\n")),(0,i.mdx)("h2",{id:"installing-from-source"},"Installing From Source"),(0,i.mdx)("p",null,"To install from source, the first step is to clone the git repository:"),(0,i.mdx)("pre",null,(0,i.mdx)("code",{parentName:"pre"},"git clone https://github.com/facebookresearch/beanmachine.git\ncd beanmachine\npip install -e .\n")),(0,i.mdx)("p",null,"If you are a developer and plan to experiment with modifying the code, we recommend replacing the last step above with:"),(0,i.mdx)("pre",null,(0,i.mdx)("code",{parentName:"pre"},'pip install -e ".[dev]"\n')))}m.isMDXComponent=!0}}]);