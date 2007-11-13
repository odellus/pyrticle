<TeXmacs|1.0.6.11>

<style|<tuple|article|axiom|maxima>>

<\body>
  <\hide-part|1>
    <doc-data|<doc-title|Notes on PIC>>
  </hide-part|>

  <\show-part|2>
    <section|Shape function Integrals>

    <with|prog-language|axiom|prog-session|default|<\session>
      <\input|<with|color|red|<with|mode|math|\<rightarrow\>> >>
        integrate((l-r^2/l)^3*r^2,r=0..l)
      </input>

      <\output>
        <with|mode|math|math-display|true|<frac|16l<rsup|6>|315><leqno>(7)>

        <axiomtype|Union(f1: OrderedCompletion Expression Integer,...) >
      </output>

      <\input|<with|color|red|<with|mode|math|\<rightarrow\>> >>
        \;
      </input>
    </session>>

    <with|prog-language|maxima|prog-session|default|<\session>
      <\input|<with|color|red|(<with|math-font-family|rm|%i>7)
      <with|color|black|>>>
        omega(n) := 2*%pi^(n/2)/gamma(n/2)
      </input>

      <\output>
        <with|mode|math|math-display|true|<with|mode|text|font-family|tt|color|red|(<with|math-font-family|rm|%o7>)
        <with|color|black|>>\<omega\><left|(>n<right|)>:=<frac|2*\<pi\><rsup|<frac|n|2>>|\<Gamma\><left|(><frac|n|2><right|)>>>
      </output>

      <\input|<with|color|red|(<with|math-font-family|rm|%i>8)
      <with|color|black|>>>
        omega(3)
      </input>

      <\output>
        <with|mode|math|math-display|true|<with|mode|text|font-family|tt|color|red|(<with|math-font-family|rm|%o13>)
        <with|color|black|>>4*\<pi\>>
      </output>

      <\input|<with|color|red|(<with|math-font-family|rm|%i>14)
      <with|color|black|>>>
        integrate((l-r^2/l)^alpha*spherearea*r^(n-1),r,0,l)
      </input>

      <\input|<with|color|red|><with|mode|math|math-display|true|<with|mode|text|Is
      >l<with|mode|text| positive or negative?>><with|color|black|>>
        positive
      </input>

      <\input|<with|color|red|><with|mode|math|math-display|true|<with|mode|text|Is
      >n<with|mode|text| positive, negative, or zero?>><with|color|black|>>
        positive
      </input>

      <\input|<with|color|red|><with|mode|math|math-display|true|<with|mode|text|Is
      >\<alpha\>+1<with|mode|text| positive, negative, or
      zero?>><with|color|black|>>
        positive
      </input>

      <\output>
        <with|mode|math|math-display|true|<with|mode|text|font-family|tt|color|red|(<with|math-font-family|rm|%o16>)
        <with|color|black|>><frac|l<rsup|n+\<alpha\>>*\<beta\><left|(><frac|n|2>,\<alpha\>+1<right|)>*<with|math-font-family|rm|spherearea>|2>>
      </output>

      <\input|<with|color|red|(<with|math-font-family|rm|%i>17)
      <with|color|black|>>>
        \;
      </input>
    </session>>
  <|show-part>
    <section|Shape function Integrals>
  </show-part>

  <\hide-part|3>
    <section|Initial Conditions by Lorentz Transform>

    Before we start, a brief remark on notation: If we let <math|\<b-v\>> be
    a vector and <math|\<b-beta\>> another vector, then we define by

    <\eqnarray*>
      <tformat|<table|<row|<cell|\<b-v\><rsub|\<\|\|\>\<b-beta\>>>|<cell|\<assign\>>|<cell|<frac|1|\<b-beta\>\<cdot\>\<b-beta\>>\<b-beta\>(\<b-beta\>\<cdot\>\<b-v\>),>>|<row|<cell|\<b-v\><rsub|\<perp\>\<b-beta\>>>|<cell|\<assign\>>|<cell|\<b-v\>-\<b-v\><rsub|\<\|\|\>\<b-beta\>>>>>>
    </eqnarray*>

    the components of <math|\<b-v\>> parallel and orthogonal to
    <math|\<b-beta\>>. Note that

    <\enumerate-alpha>
      <item>the length of <math|\<b-beta\>> plays no role in this. If
      <math|\<b-beta\>=\<b-0\>>, we define
      <math|\<b-v\><rsub|\<\|\|\>\<b-beta\>>\<assign\>\<b-0\>>.

      <item><math|\<b-v\><rsub|\<\|\|\>\<b-beta\>>> and
      <math|\<b-v\><rsub|\<perp\>\<b-beta\>>> remain vectors.

      <item><math|\<b-v\><rsub|\<\|\|\>-\<b-beta\>>=\<b-v\><rsub|\<\|\|\>\<b-beta\>>>,
      i.e. the sign of <math|\<b-beta\>> also plays no role. Think of the
      operation as a projection onto <math|span{\<b-beta\>}>.
    </enumerate-alpha>

    The situation is a charge density <math|\<rho\>> moving with velocity
    <math|\<b-v\><rsub|0>>. We fix that <math|K> is the rest frame of that
    charge carrier, thus <math|\<b-v\>=-\<b-v\><rsub|0>>, and
    <math|K<rprime|'>> is the laboratory frame.

    According to Jackson (11.19), the coordinate Lorentz Transform from a
    frame <math|K> to a frame <math|K<rprime|'>> moving with velocity
    <math|\<b-v\>> relative to <math|K> is:

    <\eqnarray*>
      <tformat|<table|<row|<cell|x<rsub|0><rprime|'>>|<cell|=>|<cell|\<gamma\>(x<rsub|0>-\<b-beta\>\<cdot\>\<b-x\>),>>|<row|<cell|\<b-x\><rprime|'>>|<cell|=>|<cell|\<b-x\><rsub|\<perp\>\<b-beta\>>+\<gamma\>\<b-x\><rsub|\<\|\|\>\<b-beta\>>-\<gamma\>\<b-beta\>x<rsub|0>,>>>>
    </eqnarray*>

    where <math|x<rsub|0>=c*t>. Inverting the coordinate formulas, we also
    obtain

    <\eqnarray*>
      <tformat|<table|<row|<cell|x<rsub|0>>|<cell|=>|<cell|\<gamma\>(x<rsub|0><rprime|'>+\<b-beta\>\<cdot\>\<b-x\><rprime|'>),>>|<row|<cell|\<b-x\>>|<cell|=>|<cell|\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>-\<gamma\>\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'>+\<gamma\>\<b-beta\>x<rsub|0><rprime|'>.>>>>
    </eqnarray*>

    We only care about time <math|t<rprime|'>=0> in <math|K>, and we assume
    that the situation in <math|K> is in steady-state, i.e.
    <math|f(\<b-x\>,x<rsub|0>)=f(\<b-x\>,0)>. Therefore, wishing to transport
    a function from <math|K> to <math|K<rprime|'>>, we find

    <\eqnarray*>
      <tformat|<table|<row|<cell|f<rprime|'>(\<b-x\><rprime|'>,0)>|<cell|=>|<cell|f(\<b-x\>,x<rsub|0>)>>|<row|<cell|>|<cell|=>|<cell|f(\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>-\<gamma\>\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'>,x<rsub|0>)>>|<row|<cell|>|<cell|=>|<cell|f(\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>-\<gamma\>\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'>,0).>>>>
    </eqnarray*>

    On the other hand,

    <\eqnarray*>
      <tformat|<table|<row|<cell|f(\<b-x\>,0)>|<cell|=>|<cell|f(\<b-x\>,\<b-beta\>\<cdot\>\<b-x\>)>>|<row|<cell|>|<cell|=>|<cell|f<rprime|'>(\<b-x\><rprime|'>,[\<b-beta\>\<cdot\>\<b-x\>]<rsub|x<rsub|0>\<rightarrow\>x<rsub|0><rprime|'>>)>>|<row|<cell|>|<cell|=>|<cell|f<rprime|'>(\<b-x\><rsub|\<perp\>\<b-beta\>>+\<gamma\>\<b-x\><rsub|\<\|\|\>\<b-beta\>>-\<gamma\>\<b-beta\>(\<b-beta\>\<cdot\>\<b-x\>),0)>>|<row|<cell|>|<cell|=>|<cell|f<rprime|'>(\<b-x\><rsub|\<perp\>\<b-beta\>>+\<gamma\>(1-\<beta\><rsup|2>)\<b-x\><rsub|\<\|\|\>\<b-beta\>>,0)>>|<row|<cell|>|<cell|=>|<cell|f<rprime|'>(\<b-x\><rsub|\<perp\>\<b-beta\>>+<frac|\<b-x\><rsub|\<\|\|\>\<b-beta\>>|\<gamma\>>,0).>>>>
    </eqnarray*>

    We may therefore neglect time dependency in the following.

    In <math|K>, <math|\<b-E\>> can be found as <math|\<nabla\>\<Phi\>> from
    <math|\<Delta\>\<Phi\>(\<b-x\>)=\<rho\>(\<b-x\>)/\<varepsilon\>>.
    <math|\<rho\><rprime|'>(\<b-x\><rprime|'>)> is given to us. Now,
    <math|c\<rho\>> is part of the 4-vector
    <math|J<rsup|\<alpha\>>=(c\<rho\>,\<b-J\>)>, therefore transforms in
    timelike fashion, and we obtain

    <\equation*>
      \<rho\><rprime|'>(\<b-x\><rprime|'>)=\<gamma\>\<rho\>(\<b-x\>).
    </equation*>

    Define

    <\eqnarray*>
      <tformat|<table|<row|<cell|<wide|\<b-E\>|~>(\<b-x\><rprime|'>)>|<cell|\<assign\>>|<cell|\<b-E\>(\<b-x\>)=\<b-E\><left|(>\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>-\<gamma\>\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'><right|)>,>>|<row|<cell|<wide|\<Phi\>|~>(\<b-x\><rprime|'>)>|<cell|\<assign\>>|<cell|\<Phi\>(\<b-x\>)=\<Phi\>(\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>-\<gamma\>\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'>),>>|<row|<cell|<wide|\<rho\>|~>(\<b-x\><rprime|'>)>|<cell|\<assign\>>|<cell|\<rho\>(\<b-x\>)=\<rho\>(\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>-\<gamma\>\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'>),>>>>
    </eqnarray*>

    i.e. the tilde'd quantities have their <math|\<b-x\>>-dependencies
    switchted to the <math|K<rprime|'>> frame, but still carry the values
    they do in <math|K>, i.e. no Lorentz transform has taken place.

    Therefore

    <\eqnarray*>
      <tformat|<table|<row|<cell|\<nabla\><rsub|\<b-x\>>\<Phi\>(\<b-x\>)>|<cell|=>|<cell|\<nabla\><rsub|\<b-x\>>[<wide|\<Phi\>|~>(\<b-x\><rprime|'>(\<b-x\>))]>>|<row|<cell|>|<cell|=>|<cell|(\<nabla\><wide|\<Phi\>|~>)(\<b-x\><rprime|'>)\<cdot\>(\<nabla\>\<b-x\><rprime|'>)(\<b-x\>)>>|<row|<cell|>|<cell|=>|<cell|(\<nabla\><wide|\<Phi\>|~>)(\<b-x\><rprime|'>)\<cdot\><left|[>Id<rsub|\<perp\>\<b-beta\>>+<frac|Id<rsub|\<\|\|\>\<b-beta\>>|\<gamma\>><right|]>>>|<row|<cell|>|<cell|=>|<cell|<left|(>\<nabla\><rsub|\<b-x\><rprime|'><rsub|\<perp\>\<b-beta\>>>+<frac|\<nabla\><rsub|\<b-x\><rprime|'><rsub|\<\|\|\>\<b-beta\>>>|\<gamma\>><right|)><wide|\<Phi\>|~>(\<b-x\><rprime|'>).>>>>
    </eqnarray*>

    We obtain

    <\eqnarray*>
      <tformat|<table|<row|<cell|\<Delta\><rsub|\<b-x\>>\<Phi\>(\<b-x\>)>|<cell|=>|<cell|<frac|\<rho\>(\<b-x\>)|\<varepsilon\>>>>|<row|<cell|\<\|\|\>>|<cell|>|<cell|<space|1em>\<\|\|\>>>|<row|<cell|<left|(>\<Delta\><rsub|\<b-x\><rprime|'><rsub|\<perp\>\<b-beta\>>>+<frac|\<Delta\><rsub|\<b-x\><rprime|'><rsub|\<\|\|\>\<b-beta\>>>|\<gamma\><rsup|2>><right|)><wide|\<Phi\>|~>(\<b-x\><rprime|'>)>|<cell|=>|<cell|<frac|<wide|\<rho\>|~>(\<b-x\><rprime|'>)|\<varepsilon\>>,>>>>
    </eqnarray*>

    and we can compute <math|<wide|\<Phi\>|~>(\<b-x\><rprime|'>)>. Next, we
    have

    <\eqnarray*>
      <tformat|<table|<row|<cell|\<b-E\>(\<b-x\>)>|<cell|=>|<cell|\<nabla\><rsub|\<b-x\>>\<Phi\>(\<b-x\>)>>|<row|<cell|\<\|\|\>>|<cell|>|<cell|\<\|\|\>>>|<row|<cell|<wide|\<b-E\>|~>(\<b-x\><rprime|'>)>|<cell|=>|<cell|<left|(>\<nabla\><rsub|\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>>+<frac|\<nabla\><rsub|\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'>>|\<gamma\>><right|)><wide|\<Phi\>|~>(\<b-x\><rprime|'>).>>>>
    </eqnarray*>

    According to Jackson (11.149), the Lorentz Transform for transforming
    <math|\<b-E\>>- and <math|\<b-B\>> in Gaussian units, reads:

    <\eqnarray*>
      <tformat|<table|<row|<cell|\<b-E\><rprime|'>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\>(\<b-E\>*(\<b-x\>,t)+\<b-beta\>\<times\>\<b-B\>(\<b-x\>,t))-<frac|\<gamma\><rsup|2>|\<gamma\>+1>\<b-beta\>(\<b-beta\>\<cdot\>\<b-E\>(\<b-x\>,t)),>>|<row|<cell|\<b-B\><rprime|'>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\>(\<b-B\>(\<b-x\>,t)-\<b-beta\>\<times\>\<b-E\>(\<b-x\>,t))-<frac|\<gamma\><rsup|2>|\<gamma\>+1>\<b-beta\>(\<b-beta\>\<cdot\>\<b-B\>(x,t)).>>>>
    </eqnarray*>

    Using the identity

    <\equation*>
      \<gamma\>-<frac|\<gamma\><rsup|2>\<beta\><rsup|2>|\<gamma\>+1>=<frac|\<gamma\><rsup|2>+\<gamma\>-\<gamma\><rsup|2>\<beta\><rsup|2>|\<gamma\>+1>=<frac|\<gamma\><rsup|2>(1-\<beta\><rsup|2>)+\<gamma\>|\<gamma\>+1>=<frac|1+\<gamma\>|\<gamma\>+1>=1,
    </equation*>

    we find the (much simpler) expressions

    <\eqnarray*>
      <tformat|<table|<row|<cell|\<b-E\><rprime|'>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\>(\<b-E\>*<rsub|\<perp\>\<b-beta\>>(\<b-x\>,t)+\<b-beta\>\<times\>\<b-B\>(\<b-x\>,t))+\<b-E\><rsub|\<\|\|\>\<b-beta\>>(\<b-x\>,t),>>|<row|<cell|\<b-B\><rprime|'>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\>(\<b-B\><rsub|\<perp\>\<b-beta\>>(\<b-x\>,t)-\<b-beta\>\<times\>\<b-E\>(\<b-x\>,t))+\<b-B\><rsub|\<\|\|\>\<b-beta\>>(x,t).>>>>
    </eqnarray*>

    Converted to SI, we get

    <\eqnarray*>
      <tformat|<table|<row|<cell|<sqrt|\<varepsilon\><rsub|0>>\<b-E\><rprime|'>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\>(<sqrt|\<varepsilon\><rsub|0>>\<b-E\><rsub|\<perp\>\<b-beta\>>*(\<b-x\>,t)+\<b-beta\>\<times\><frac|\<b-B\>|<sqrt|\<mu\><rsub|0>>>(\<b-x\>,t))+<sqrt|\<varepsilon\><rsub|0>>\<b-E\><rsub|\<\|\|\>\<b-beta\>>(\<b-x\>,t),>>|<row|<cell|<frac|\<b-B\><rprime|'>|<sqrt|\<mu\><rsub|0>>>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\><left|(><frac|\<b-B\><rsub|\<perp\>\<b-beta\>>|<sqrt|\<mu\><rsub|0>>>(\<b-x\>,t)-\<b-beta\>\<times\><sqrt|\<varepsilon\><rsub|0>>\<b-E\>(\<b-x\>,t)<right|)>+<frac|\<b-B\><rsub|\<\|\|\>\<b-beta\>>|<sqrt|\<mu\><rsub|0>>>(x,t)<right|)>>>>>
    </eqnarray*>

    or

    <\eqnarray*>
      <tformat|<table|<row|<cell|\<b-E\><rprime|'>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\>(\<b-E\><rsub|\<perp\>\<b-beta\>>*(\<b-x\>,t)+c\<b-beta\>\<times\>\<b-B\>(\<b-x\>,t))-\<b-E\><rsub|\<\|\|\>\<b-beta\>>(\<b-x\>,t),>>|<row|<cell|\<b-B\><rprime|'>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\><left|(>\<b-B\><rsub|\<perp\>\<b-beta\>>(\<b-x\>,t)-<frac|\<b-beta\>|c>\<times\>\<b-E\>(\<b-x\>,t)<right|)>-\<b-B\><rsub|\<\|\|\>\<b-beta\>>(x,t).>>>>
    </eqnarray*>

    Next, observe that the situation in the rest frame is purely
    electrostatic, i.e. there is no time dependency and no magnetic field.
    This simplifies the field Lorentz transform to

    <\eqnarray*>
      <tformat|<table|<row|<cell|\<b-E\><rprime|'>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\>\<b-E\><rsub|\<perp\>\<b-beta\>>(\<b-x\>)+\<b-E\><rsub|\<\|\|\>\<b-beta\>>(\<b-x\>),>>|<row|<cell|\<b-B\><rprime|'>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\><frac|\<b-beta\>|c>\<times\>\<b-E\>(\<b-x\>),>>>>
    </eqnarray*>

    or, using <math|<wide|\<b-E\>|~>>, to the final form

    <\eqnarray*>
      <tformat|<table|<row|<cell|\<b-E\><rprime|'>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\><wide|\<b-E\>|~><rsub|\<perp\>\<b-beta\>>(\<b-x\><rprime|'>)+<wide|\<b-E\>|~><rsub|\<\|\|\>\<b-beta\>>(\<b-x\><rprime|'>),>>|<row|<cell|\<b-B\><rprime|'>(\<b-x\><rprime|'>,t<rprime|'>)>|<cell|=>|<cell|\<gamma\><frac|\<b-beta\>|c>\<times\><wide|\<b-E\>|~>(\<b-x\><rprime|'>).>>>>
    </eqnarray*>

    It remains to verify that this modified initial condition actually
    satisfies <math|div<rsub|\<b-x\><rprime|'>>\<b-E\><rprime|'>=\<rho\><rprime|'>/\<varepsilon\>>.

    <\eqnarray*>
      <tformat|<table|<row|<cell|div<rsub|\<b-x\><rprime|'>>\<b-E\><rprime|'>(\<b-x\><rprime|'>)>|<cell|=>|<cell|div<rsub|\<b-x\><rprime|'>><left|[>\<gamma\><wide|\<b-E\>|~><rsub|\<perp\>\<b-beta\>>(\<b-x\><rprime|'>)+<wide|\<b-E\>|~><rsub|\<\|\|\>\<b-beta\>>(\<b-x\><rprime|'>)<right|]>>>|<row|<cell|>|<cell|=>|<cell|div<rsub|\<b-x\><rprime|'>><left|[>\<gamma\>\<nabla\><rsub|\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>><wide|\<Phi\>|~>(\<b-x\><rprime|'>)+<frac|\<nabla\><rsub|\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'>>|\<gamma\>><wide|\<Phi\>|~>(\<b-x\><rprime|'>)<right|]>>>|<row|<cell|>|<cell|=>|<cell|\<gamma\>\<nabla\><rsub|\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>>\<cdot\>(\<nabla\><rsub|\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>><wide|\<Phi\>|~>(\<b-x\><rprime|'>))+<frac|\<nabla\><rsub|\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'>>\<cdot\>(\<nabla\><rsub|\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'>><wide|\<Phi\>|~>(\<b-x\><rprime|'>))|\<gamma\>>>>|<row|<cell|>|<cell|=>|<cell|\<gamma\><left|[>\<nabla\><rsub|\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>>\<cdot\>(\<nabla\><rsub|\<b-x\><rsub|\<perp\>\<b-beta\>><rprime|'>><wide|\<Phi\>|~>(\<b-x\><rprime|'>))+<frac|\<nabla\><rsub|\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'>>\<cdot\>(\<nabla\><rsub|\<b-x\><rsub|\<\|\|\>\<b-beta\>><rprime|'>><wide|\<Phi\>|~>(\<b-x\><rprime|'>))|\<gamma\><rsup|2>><right|]>>>|<row|<cell|>|<cell|=>|<cell|\<gamma\><left|[><frac|<wide|\<rho\>|~>(\<b-x\><rprime|'>)|\<varepsilon\>><right|]>=<frac|\<rho\><rprime|'>(\<b-x\><rprime|'>)|\<varepsilon\>>.>>>>
    </eqnarray*>
  <|hide-part>
    <section|Initial Conditions by Lorentz Transform>
  </hide-part>

  <\hide-part|4>
    <section|Relativistic Momentum>

    <with|prog-language|axiom|prog-session|default|<\session>
      <\input|<with|color|red|<with|mode|math|\<rightarrow\>> >>
        gamma:=1/sqrt(1-v^2/c^2)
      </input>

      <\output>
        <with|mode|math|<frac|<with|math-display|true|1>|<sqrt|<frac|-v<rsup|2>+c<rsup|2>|c<rsup|2>>>><leqno>(1)>

        <axiomtype|Expression Integer >
      </output>

      <\input|<with|color|red|<with|mode|math|\<rightarrow\>> >>
        solve(m*gamma*v=p,v)
      </input>

      <\output>
        <with|mode|math|math-display|true|<left|[>v=<frac|c*p|<sqrt|p<rsup|2>+c<rsup|2>m<rsup|2>>>,<space|0.5spc>v=-<frac|c*p|<sqrt|p<rsup|2>+c<rsup|2>m<rsup|2>>><right|]><leqno>(2)>

        <axiomtype|List Equation Expression Integer >
      </output>

      <\input|<with|color|red|<with|mode|math|\<rightarrow\>> >>
        \;
      </input>
    </session>>
  <|hide-part>
    <section|Relativistic Momentum>
  </hide-part>
</body>

<\initial>
  <\collection>
    <associate|page-type|letter>
  </collection>
</initial>

<\references>
  <\collection>
    <associate|auto-1|<tuple|1|1>>
    <associate|auto-2|<tuple|2|1>>
    <associate|auto-3|<tuple|3|?>>
    <associate|auto.2-1|<tuple|1|?|#2>>
    <associate|auto.3-1|<tuple|2|?|#3>>
    <associate|auto.3-2|<tuple|3|?|#3>>
    <associate|auto.4-1|<tuple|3|?|#4>>
  </collection>
</references>

<\auxiliary>
  <\collection>
    <\associate|toc>
      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|1<space|2spc>Shape
      function Integrals> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-1><vspace|0.5fn>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|2<space|2spc>Initial
      Conditions by Lorentz Transform> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-2><vspace|0.5fn>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|3<space|2spc>Relativistic
      Momentum> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-3><vspace|0.5fn>
    </associate>
  </collection>
</auxiliary>