
#ifndef  integration_h
#define  integration_h


// ====================================================
//                    Adaptive Simpson
// ====================================================



namespace AdaptiveIntegral{


double trapezRecur( double (*func)(double), double xa, double xb, double fa, double fb, double Iab, int depth, double eps ){
    double h4   = (xb-xa)*.25;
    double xm   = (xa+xb)*.5;
    double fm   = func(xm);
    double Iam  = (fa+fm)*h4;
    double Imb  = (fm+fb)*h4;
    double Iamb = Iam+Imb;
    double delta = Iab-Iamb;
    // ToDo: there is no higher order correction like for Sympson ?
    //printf( "err : %g   | [%g,%g] depth %i \n", delta, xa,xb, depth );
    if( depth <= 0 || fabs(delta) < 3*eps ){ return Iamb; }
    eps*=.5;
    depth--;
    return trapezRecur( func, xa, xm, fa, fm, Iam, depth, eps )
        +  trapezRecur( func, xm, xb, fm, fb, Imb, depth, eps );
}

double trapez(
    double (*func)(double),        // function ptr to integrate
    double xa, double xb,         // interval [a,b]
    double eps,             // error tolerance
    int depth                // recursion cap
){
    //errno = 0;
    double h = xb - xa;
    if (h == 0) return 0;
    double fa = func(xa), fb = func(xb);
    double Iab = (h/2)*(fa + fb);
    return trapezRecur( func, xa, xb, fa, fb, Iab, depth, eps );
}



double simpsonRecur(
    double (*func)(double),
    double xa, double xb, double Iab,
    double fa, double fb, double fm,
    int depth, double eps
){
    //double m   = (a + b)/2,  h   = (b - a)/2;
    //double lm  = (a + m)/2,  rm  = (m + b)/2;
    double xm   = (xa + xb)*.5,  h6  = (xb - xa)*0.08333333333;
    double xlm  = (xa + xm)*.5,  xrm = (xm + xb)*.5;
    // serious numerical trouble: it won't converge
    //if ((eps/2 == eps) || (a == lm)) { errno = EDOM; return whole; }
    double flm   = func(xlm),      frm = func(xrm);
    double Iam   = h6 * (fa + 4*flm + fm);
    double Imb   = h6 * (fm + 4*frm + fb);
    double delta = Iam + Imb - Iab;
    //if (rec <= 0 && errno != EDOM) errno = ERANGE;  // depth limit too shallow
    // Lyness 1969 + Richardson extrapolation; see article
    //if ( depth <= 0 || fabs(delta) <= 15*eps) return Iam + Imb + (delta)/15;
    //printf( "err : %g   | [%g,%g] depth %i \n", delta, xa,xb, depth );
    if ( depth <= 0 || fabs(delta) <= 15*eps) return Iam + Imb + delta*0.06666666666;
    depth--;
    eps*=.5;
    return simpsonRecur(func, xa, xm, Iam, fa, fm, flm, depth, eps )
         + simpsonRecur(func, xm, xb, Imb, fm, fb, frm, depth, eps );
}

double simpson(
    double (*func)(double),        // function ptr to integrate
    double xa, double xb,         // interval [a,b]
    double eps,             // error tolerance
    int depth                // recursion cap
){
    //errno = 0;
    double h = xb - xa;
    if (h == 0) return 0;
    double fa = func(xa), fb = func(xb), fm = func((xa + xb)/2);
    double Iab = (h/6)*(fa + 4*fm + fb);
    return simpsonRecur(func, xa, xb, Iab, fa, fb, fm, depth, eps);
}


}; // namespace AdaptiveSimpson



// ====================================================
//                    Newton-Cotes
// ====================================================
// http://en.wikipedia.org/wiki/Newton%E2%80%93Cotes_formulas
// http://people.oregonstate.edu/~peterseb/mth351/docs/newton_cotes.pdf

namespace NewtonCotes {
constexpr static const double ws_2 []= {    1/2.0,          1/2.0  };
constexpr static const double ws_3 []= {    1/6.0,          2/3.0,           1/6.0 };
constexpr static const double ws_4 []= {    1/8.0,          3/8.0,           3/8.0,         1/8.0  };
constexpr static const double ws_5 []= {    7/90.0,        16/45.0,          2/15.0,       16/45.0,       7/90.0  };
constexpr static const double ws_6 []= {   19/288.0,       25/96.0,         25/144.0,      25/144.0,     25/96.0,       19/288.0 };
constexpr static const double ws_7 []= {   41/840.0,        9/35.0,          9/280.0,      34/105.0,      9/280.0,       9/35.0,       41/840.0   };
constexpr static const double ws_8 []= {  751/17280.0,   3577/17280.0,      49/640.0,    2989/17280.0, 2989/17280.0,    49/640.0,    3577/17280.0,  751/17280.0  };
constexpr static const double ws_9 []= {  989/28350.0,   2944/14175.0,    -464/14175.0,  5248/14175.0, -454/2835.0,   5248/14175.0,  -464/14175.0, 2944/14175.0,     989/28350.0 };
constexpr static const double ws_10[]= { 2857/89600.0,  15741/89600.0,      27/2240.0,   1209/5600.0,  2889/44800.0,  2889/44800.0,  1209/5600.0,    27/2240.0,    15741/89600.0,   2857/89600.0 };
constexpr static const double ws_11[]= {16067/598752.0, 26575/149688.0, -16175/199584.0, 5675/12474.0,  -4825/11088.0, 17807/24948.0, -4825/11088.0, 5675/12474.0,  -16175/199584.0, 26575/149688.0, 16067/598752.0 };
}; // namespace NewtonCotes

// ====================================================
//              Legendre-Gauss Quadrature
// ====================================================
// http://en.wikipedia.org/wiki/Gaussian_quadrature
// http://www.scientificpython.net/1/post/2012/04/gausslegendre1.html

namespace GaussQuadrature {

constexpr static const double xs_3 [] ={0.8872983346207417,  0.49999999999999994, 0.1127016653792583 };
constexpr static const double xs_4 [] ={0.93056815579702623, 0.66999052179242802, 0.33000947820757176, 0.069431844202973714 };
constexpr static const double xs_5 [] ={0.95308992296933193, 0.7692346550528415,  0.49999999999999994, 0.23076534494715839, 0.046910077030667963 };
constexpr static const double xs_6 [] ={0.96623475710157603, 0.83060469323313224, 0.61930959304159838, 0.38069040695840151, 0.1693953067668677, 0.033765242898423975 };
constexpr static const double xs_7 [] ={0.97455395617137919, 0.87076559279969723, 0.70292257568869854, 0.50000000000000000, 0.29707742431130141, 0.12923440720030277, 0.025446043828620757 };
constexpr static const double xs_8 [] ={0.9801449282487682,  0.89833323870681336, 0.7627662049581645,  0.59171732124782483, 0.40828267875217505, 0.23723379504183550, 0.101666761293186640, 0.019855071751231856 };
constexpr static const double xs_9 [] ={0.98408011975381304, 0.91801555366331788, 0.80668571635029518, 0.66212671170190440, 0.50000000000000000, 0.33787328829809554, 0.193314283649704820, 0.081984446336682115, 0.015919880246186957 };
constexpr static const double xs_10[] ={0.98695326425858587, 0.93253168334449221, 0.83970478414951222, 0.71669769706462350, 0.57443716949081558, 0.42556283050918436, 0.283302302935376390, 0.160295215850487780, 0.067468316655507732, 0.013046735741414128 };
constexpr static const double xs_11[] ={0.98911432907302843, 0.94353129988404771, 0.86507600278702468, 0.75954806460340585, 0.63477157797617245, 0.50000000000000000, 0.365228422023827490, 0.240451935396594100, 0.134923997212975320, 0.056468700115952342, 0.010885670926971514 };
constexpr static const double xs_12[] ={0.99078031712335957, 0.95205862818523745, 0.88495133709715224, 0.79365897714330869, 0.68391574949909006, 0.56261670425573440, 0.437383295744265490, 0.316084250500909880, 0.206341022856691260, 0.115048662902847600, 0.047941371814762546, 0.0092196828766403782 };
constexpr static const double xs_13[] ={0.99209152735929407, 0.95879919961148896, 0.90078904536665494, 0.82117466972017006, 0.72424637551822335, 0.61522915797756739, 0.500000000000000000, 0.384770842022432610, 0.275753624481776540, 0.178825330279829890, 0.099210954633345005, 0.0412008003885110390, 0.007908472640705877 };
constexpr static const double xs_14[] ={0.99314190434840621, 0.96421744183178681, 0.91360065753488251, 0.84364645240584268, 0.75762431817907705, 0.65955618446394482, 0.554027474353671830, 0.445972525646328170, 0.340443815536055130, 0.242375681820922950, 0.156353547594157260, 0.0863993424651174900, 0.035782558168213241, 0.0068580956515938429 };

//constexpr static const double xs** = { 0,0,0,xs_3, xs_4, xs_5, xs_6, xs_7, xs_8, xs_9, xs_10, xs_11, xs_12, xs_13, xs_14 };


constexpr static const double ws_3 [] ={0.277777777777778350, 0.444444444444444420, 0.277777777777777240 };
constexpr static const double ws_4 [] ={0.173927422568728200, 0.326072577431273380, 0.326072577431272710, 0.173927422568725900 };
constexpr static const double ws_5 [] ={0.118463442528095390, 0.239314335249683570, 0.284444444444444440, 0.239314335249683100, 0.118463442528093930 };
constexpr static const double ws_6 [] ={0.085662246189585275, 0.180380786524069440, 0.233956967286345410, 0.233956967286345550, 0.180380786524069080, 0.085662246189584831 };
constexpr static const double ws_7 [] ={0.064742483084435212, 0.139852695744638430, 0.190915025252559490, 0.208979591836734700, 0.190915025252559410, 0.139852695744638410, 0.064742483084434546 };
constexpr static const double ws_8 [] ={0.050614268145188400, 0.111190517226687460, 0.156853322938943720, 0.181341891689181000, 0.181341891689181000, 0.156853322938943630, 0.111190517226687200, 0.050614268145187803 };
constexpr static const double ws_9 [] ={0.040637194180787546, 0.090324080347429100, 0.130305348201467910, 0.156173538520001490, 0.165119677500629890, 0.156173538520001400, 0.130305348201467720, 0.090324080347428518, 0.040637194180786686 };
constexpr static const double ws_10[] ={0.033335672154344714, 0.074725674575291237, 0.109543181257991480, 0.134633359654998420, 0.147762112357376490, 0.147762112357376410, 0.134633359654998090, 0.109543181257990740, 0.074725674575289669, 0.033335672154342910 };
constexpr static const double ws_11[] ={0.027834283558087185, 0.062790184732452473, 0.093145105463867298, 0.116596882295995340, 0.131402272255123380, 0.136462543388950310, 0.131402272255123270, 0.116596882295995230, 0.093145105463867020, 0.062790184732452195, 0.027834283558086554 };
constexpr static const double ws_12[] ={0.023587668193256826, 0.053469662997660185, 0.080039164271673749, 0.101583713361533280, 0.116746268269177620, 0.124573522906701460, 0.124573522906701360, 0.116746268269177330, 0.101583713361532800, 0.080039164271672750, 0.053469662997658561, 0.023587668193254235 };
constexpr static const double ws_13[] ={0.020242002382658223, 0.046060749918864691, 0.069436755109893944, 0.089072990380973063, 0.103908023768444320, 0.113141590131448700, 0.116275776615436950, 0.113141590131448600, 0.103908023768444160, 0.089072990380972730, 0.069436755109893500, 0.046060749918864025, 0.020242002382657165 };
constexpr static const double ws_14[] ={0.017559730165876000, 0.040079043579880277, 0.060759285343951815, 0.078601583579096815, 0.092769198738968967, 0.102599231860647800, 0.107631926731578880, 0.107631926731578880, 0.102599231860647800, 0.092769198738968897, 0.078601583579096690, 0.060759285343951600, 0.040079043579880028, 0.017559730165875635 };

//constexpr static const double ws*[] = { 0,0,0,ws_3, ws_4, ws_5, ws_6, ws_7, ws_8, ws_9, ws_10, ws_11, ws_12, ws_13, ws_14 };

/*
def GaussLegendreCoefs(N):
	# Initial approximation to roots of the Legendre polynomial
	a = linspace(3,4*N-1,N)/(4*N+2)
	x = cos(pi*a+1/(8*N*N*tan(a)))
	# Find roots using Newton's method
	epsilon = 1e-15
	#epsilon = 1e-17
	delta = 1.0
	while delta>epsilon:
		p0 = ones(N,float)
		p1 = copy(x)
		for k in range(1,N):
			p0,p1 = p1,((2*k+1)*x*p1-k*p0)/(k+1)
		dp = (N+1)*(p0-x*p1)/(1-x*x)
		dx = p1/dp
		x -= dx
		delta = max(abs(dx))
	w = 2*(N+1)*(N+1)/(N*N*(1-x*x)*dp*dp)
	return 0.5*(x+1.0),w*0.5
	#return 10*x,w
*/


}; // namespace GaussQuadrature


// ====================================================
//              Clenshaw-Curtis Quadrature
// ====================================================
// http://en.wikipedia.org/wiki/Clenshaw%E2%80%93Curtis_quadrature
// http://www.scientificpython.net/1/post/2012/04/clenshaw-curtis-quadrature.html

namespace ClenshawCurtis{

constexpr static const double xs_3 [] ={ 0.0000000000000000000, 0.500000000000000000, 1.000000000000000000 };
constexpr static const double xs_4 [] ={ 0.0000000000000000000, 0.250000000000000000, 0.750000000000000000, 1.00000000000000000 };
constexpr static const double xs_5 [] ={ 0.0000000000000000000, 0.146446609406726270, 0.500000000000000000, 0.85355339059327373, 1.00000000000000000 };
constexpr static const double xs_6 [] ={ 0.0000000000000000000, 0.095491502812526274, 0.345491502812526270, 0.65450849718747373, 0.90450849718747373, 1.00000000000000000 };
constexpr static const double xs_7 [] ={ 0.0000000000000000000, 0.066987298107780702, 0.250000000000000000, 0.50000000000000000, 0.75000000000000000, 0.93301270189221930, 1.00000000000000000 };
constexpr static const double xs_8 [] ={ 0.0000000000000000000, 0.049515566048790371, 0.188255099070633200, 0.38873953302184272, 0.61126046697815728, 0.81174490092936680, 0.95048443395120963, 1.00000000000000000 };
constexpr static const double xs_9 [] ={ 0.0000000000000000000, 0.038060233744356631, 0.146446609406726210, 0.30865828381745508, 0.50000000000000000, 0.69134171618254492, 0.85355339059327373, 0.96193976625564337, 1.00000000000000000 };
constexpr static const double xs_10[] ={ 0.0000000000000000000, 0.030153689607045786, 0.116977778440511050, 0.25000000000000000, 0.41317591116653485, 0.58682408883346515, 0.75000000000000000, 0.88302222155948895, 0.96984631039295421, 1.00000000000000000 };
constexpr static const double xs_11[] ={ 0.0000000000000000000, 0.024471741852423179, 0.095491502812526274, 0.20610737385376338, 0.34549150281252627, 0.50000000000000000, 0.65450849718747373, 0.79389262614623668, 0.90450849718747373, 0.97552825814757682, 1.00000000000000000 };
constexpr static const double xs_12[] ={ 0.0000000000000000000, 0.020253513192751260, 0.079373233584409397, 0.17256963302735745, 0.29229249349905678, 0.42884258086335736, 0.57115741913664264, 0.70770750650094327, 0.82743036697264261, 0.92062676641559060, 0.97974648680724874, 1.0000000000000000 };
constexpr static const double xs_13[] ={ 0.0000000000000000000, 0.017037086855465844, 0.066987298107780702, 0.14644660940672621, 0.25000000000000000, 0.37059047744873963, 0.50000000000000000, 0.62940952255126037, 0.75000000000000000, 0.85355339059327373, 0.93301270189221930, 0.9829629131445341, 1.00000000000000000 };
constexpr static const double xs_14[] ={ 0.0000000000000000000, 0.014529091286973994, 0.057271987173395045, 0.12574462591444940, 0.21596762663442204, 0.32269755647873211, 0.43973165987233842, 0.56026834012766158, 0.67730244352126789, 0.78403237336557796, 0.87425537408555054, 0.9427280128266049, 0.98547090871302601, 1.00000000000000000 };

//constexpr static const double xs*[] = { 0,0,0,xs_3, xs_4, xs_5, xs_6, xs_7, xs_8, xs_9, xs_10, xs_11, xs_12, xs_13, xs_14 };

constexpr static const double ws_3 [] ={ 0.1666666666666666900, 0.666666666666666630, 0.166666666666666690 };
constexpr static const double ws_4 [] ={ 0.0555555555555555590, 0.444444444444444420, 0.444444444444444420, 0.055555555555555559 };
constexpr static const double ws_5 [] ={ 0.0333333333333333400, 0.266666666666666660, 0.400000000000000020, 0.266666666666666660, 0.033333333333333340 };
constexpr static const double ws_6 [] ={ 0.0200000000000000040, 0.180371520600005610, 0.299628479399994370, 0.299628479399994370, 0.180371520600005610, 0.020000000000000004} ;
constexpr static const double ws_7 [] ={ 0.0142857142857142900, 0.126984126984126980, 0.228571428571428560, 0.260317460317460280, 0.228571428571428560, 0.12698412698412698, 0.014285714285714290 };
constexpr static const double ws_8 [] ={ 0.0102040816326530640, 0.095070503609104157, 0.176121211859079560, 0.218604202899163190, 0.218604202899163190, 0.17612121185907956, 0.095070503609104157, 0.010204081632653064 };
constexpr static const double ws_9 [] ={ 0.0079365079365079430, 0.073109324608009077, 0.139682539682539690, 0.180858929360244890, 0.196825396825396850, 0.18085892936024489, 0.139682539682539690, 0.073109324608009077, 0.007936507936507943 };
constexpr static const double ws_10[] ={ 0.0061728395061728418, 0.058283728286018549, 0.112642161669052190, 0.150970017636684300, 0.171931252902072030, 0.17193125290207203, 0.150970017636684300, 0.112642161669052190, 0.058283728286018549, 0.0061728395061728418 };
constexpr static const double ws_11[] ={ 0.0050505050505050553, 0.047289527441850776, 0.092817607212123912, 0.126794166641843310, 0.149606635212118560, 0.1568831168831169, 0.1496066352121185600, 0.126794166641843310, 0.092817607212123912, 0.0472895274418507760, 0.0050505050505050553 };
constexpr static const double ws_12[] ={ 0.0041322314049586795, 0.039280076873099999, 0.077520227541280684, 0.107781273000434290, 0.129958670533458080, 0.14132752064676826, 0.141327520646768260, 0.129958670533458080, 0.107781273000434290, 0.0775202275412806840, 0.0392800768730999990, 0.0041322314049586795 };
constexpr static const double ws_13[] ={ 0.0034965034965034982, 0.033028712476037198, 0.065771265771265769, 0.092381692381692374, 0.113486513486513480, 0.12633784689052213, 0.130994930994930980, 0.126337846890522130, 0.113486513486513480, 0.0923816923816923740, 0.0657712657712657690, 0.0330287124760371980, 0.0034965034965034982 };
constexpr static const double ws_14[] ={ 0.0029585798816568068, 0.028232656881707230, 0.056384336244928279, 0.080019013058359331, 0.099496205182891623, 0.11295152488928223, 0.119957683861174520, 0.119957683861174520, 0.112951524889282230, 0.0994962051828916230, 0.0800190130583593310, 0.0563843362449282790, 0.0282326568817072300, 0.0029585798816568068 };

//double ws*[] = { 0,0,0,ws_3, ws_4, ws_5, ws_6, ws_7, ws_8, ws_9, ws_10, ws_11, ws_12, ws_13, ws_14 };

/*
def ClenshawCurtisCoefs(n1):
	if n1 == 1:
		x = 0
		w = 2
	else:
		n = n1 - 1
		C = zeros((n1,2))
		k = 2*(1+arange(np.floor(n/2)))
		C[::2,0] = 2/hstack((1, 1-k*k))
		C[1,1] = -n
		V = vstack((C,flipud(C[1:n,:])))
		F = real(ifft(V, n=None, axis=0))
		x = F[0:n1,1]
		w = hstack((F[0,0],2*F[1:n,0],F[n,0]))
	#return x,w
	return 0.5*(x+1.0),0.5*w
*/

}; // namespace ClenshawCurtis

#endif

