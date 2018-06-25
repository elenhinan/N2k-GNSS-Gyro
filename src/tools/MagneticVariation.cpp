#include <math.h>
#include "MagneticVariation.h"
#include "WMM2015cof.h"

const double wmmtest[12][5] = {
  //   year   alt   lat   lon     dec
    { 2015.0,   0,   80,    0,   -3.85},
    { 2015.0,   0,    0,  120,    0.57},
    { 2015.0,   0,  -80,  240,   69.81},
    { 2015.0, 100,   80,    0,   -4.27},
    { 2015.0, 100,    0,  120,    0.56},
    { 2015.0, 100,  -80,  240,   69.81},
    { 2017.5,   0,   80,    0,   -2.75},
    { 2017.5,   0,    0,  120,    0.32},
    { 2017.5,   0,  -80,  240,   69.58},
    { 2017.5, 100,   80,    0,   -3.17},
    { 2017.5, 100,    0,  120,    0.32},
    { 2017.5, 100,  -80,  240,   69.00}
  };

void MagneticVariationTest(Stream *output) {
  double dec, dip, ti, gv;
  output->print("Testing ");
  output->print(WMMmodel);
  output->print(" model");
  for(int i=0; i<12; i++) {
      MagneticVariation(wmmtest[i][1]*1000.0, wmmtest[i][2], wmmtest[i][3], wmmtest[i][0], dec, dip, ti, gv);
    output->print("i: ");
    output->print(i);
    output->print(" should be: ");
    output->print(wmmtest[i][4], 3);
    output->print(" got: ");
    output->println(dec, 3);
  }
}

bool MagneticVariation(double alt, double glat, double glon, double gtime, double &dec, double &dip, double &ti, double &gv)
{
    int maxord,n,m,D3,D4;
    double tc[13][13],dp[13][13],
        sp[13],cp[13],pp[13],pi,dtr,a,b,re,
        a2,b2,c2,a4,b4,c4,otime,oalt,
        olat,olon,dt,rlon,rlat,srlon,srlat,crlon,crlat,srlat2,
        crlat2,q,q1,q2,ct,st,r2,r,d,ca,sa,aor,ar,br,bt,bp,bpp,
        par,temp1,temp2,parp,bx,by,bz,bh;
    double *p = snorm;

    // initialize to prevent warint
    sa = ca = r = st = ct = 0.0;

    // convert altitude from meters to km needed in model
    alt *= 0.001;

    /* INITIALIZE CONSTANTS */
    maxord = 12;
    sp[0] = 0.0;
    cp[0] = pp[0] = 1.0;
    dp[0][0] = 0.0;
    a = 6378.137;
    b = 6356.7523142;
    re = 6371.2;
    a2 = a*a;
    b2 = b*b;
    c2 = a2-b2;
    a4 = a2*a2;
    b4 = b2*b2;
    c4 = a4 - b4;
    otime = oalt = olat = olon = -1000.0;


    dt = gtime - epoch;
    if (dt < 0.0 || dt > 5.0)
    {
        return false;    // outside lifespan of model
    }

    pi = 3.14159265359;
    dtr = pi/180.0;
    rlon = glon*dtr;
    rlat = glat*dtr;
    srlon = sin(rlon);
    srlat = sin(rlat);
    crlon = cos(rlon);
    crlat = cos(rlat);
    srlat2 = srlat*srlat;
    crlat2 = crlat*crlat;
    sp[1] = srlon;
    cp[1] = crlon;

    /* CONVERT FROM GEODETIC COORDS. TO SPHERICAL COORDS. */
    if (alt != oalt || glat != olat)
        {
        q = sqrt(a2-c2*srlat2);
        q1 = alt*q;
        q2 = ((q1+a2)/(q1+b2))*((q1+a2)/(q1+b2));
        ct = srlat/sqrt(q2*crlat2+srlat2);
        st = sqrt(1.0-(ct*ct));
        r2 = (alt*alt)+2.0*q1+(a4-c4*srlat2)/(q*q);
        r = sqrt(r2);
        d = sqrt(a2*crlat2+b2*srlat2);
        ca = (alt+d)/r;
        sa = c2*crlat*srlat/(r*d);
        }
    if (glon != olon)
        {
        for (m=2; m<=maxord; m++)
            {
            sp[m] = sp[1]*cp[m-1]+cp[1]*sp[m-1];
            cp[m] = cp[1]*cp[m-1]-sp[1]*sp[m-1];
            }
        }
    aor = re/r;
    ar = aor*aor;
    br = bt = bp = bpp = 0.0;
    for (n=1; n<=maxord; n++)
        {
        ar = ar*aor;
        for (m=0,D3=1,D4=(n+m+D3)/D3; D4>0; D4--,m+=D3)
            {
    /*
    COMPUTE UNNORMALIZED ASSOCIATED LEGENDRE POLYNOMIALS
    AND DERIVATIVES VIA RECURSION RELATIONS
    */
            if (alt != oalt || glat != olat)
                {
                if (n == m)
                    {
                    *(p+n+m*13) = st**(p+n-1+(m-1)*13);
                    dp[m][n] = st*dp[m-1][n-1]+ct**(p+n-1+(m-1)*13);
                    goto S50;
                    }
                if (n == 1 && m == 0)
                    {
                    *(p+n+m*13) = ct**(p+n-1+m*13);
                    dp[m][n] = ct*dp[m][n-1]-st**(p+n-1+m*13);
                    goto S50;
                    }
                if (n > 1 && n != m)
                    {
                    if (m > n-2) *(p+n-2+m*13) = 0.0;
                    if (m > n-2) dp[m][n-2] = 0.0;
                    *(p+n+m*13) = ct**(p+n-1+m*13)-k[m][n]**(p+n-2+m*13);
                    dp[m][n] = ct*dp[m][n-1] - st**(p+n-1+m*13)-k[m][n]*dp[m][n-2];
                    }
                }
            S50:
    /*
        TIME ADJUST THE GAUSS COEFFICIENTS
    */
            if (gtime != otime)
                {
                tc[m][n] = c[m][n]+dt*cd[m][n];
                if (m != 0) tc[n][m-1] = c[n][m-1]+dt*cd[n][m-1];
                }
    /*
        ACCUMULATE TERMS OF THE SPHERICAL HARMONIC EXPANSIONS
    */
            par = ar**(p+n+m*13);
            if (m == 0)
                {
                temp1 = tc[m][n]*cp[m];
                temp2 = tc[m][n]*sp[m];
                }
            else
                {
                temp1 = tc[m][n]*cp[m]+tc[n][m-1]*sp[m];
                temp2 = tc[m][n]*sp[m]-tc[n][m-1]*cp[m];
                }
            bt = bt-ar*temp1*dp[m][n];
            bp += (fm[m]*temp2*par);
            br += (fn[n]*temp1*par);
    /*
        SPECIAL CASE:  NORTH/SOUTH GEOGRAPHIC POLES
    */
            if (st == 0.0 && m == 1)
                {
                if (n == 1) pp[n] = pp[n-1];
                else pp[n] = ct*pp[n-1]-k[m][n]*pp[n-2];
                parp = ar*pp[n];
                bpp += (fm[m]*temp2*parp);
                }
            }
        }
    if (st == 0.0) bp = bpp;
    else bp /= st;
    /*
        ROTATE MAGNETIC VECTOR COMPONENTS FROM SPHERICAL TO
        GEODETIC COORDINATES
    */
    bx = -bt*ca-br*sa;
    by = bp;
    bz = bt*sa-br*ca;
    /*
        COMPUTE DECLINATION (DEC), INCLINATION (DIP) AND
        TOTAL INTENSITY (TI)
    */
    bh = sqrt((bx*bx)+(by*by));
    ti = sqrt((bh*bh)+(bz*bz));
    dec = atan2(by,bx)/dtr;
    dip = atan2(bz,bh)/dtr;
    /*
        COMPUTE MAGNETIC GRID VARIATION IF THE CURRENT
        GEODETIC POSITION IS IN THE ARCTIC OR ANTARCTIC
        (I.E. GLAT > +55 DEGREES OR GLAT < -55 DEGREES)

        OTHERWISE, SET MAGNETIC GRID VARIATION TO -999.0
    */
    gv = -999.0;
    if (fabs(glat) >= 55.)
        {
        if (glat > 0.0 && glon >= 0.0) gv = dec-glon;
        if (glat > 0.0 && glon < 0.0) gv = dec+fabs(glon);
        if (glat < 0.0 && glon >= 0.0) gv = dec+glon;
        if (glat < 0.0 && glon < 0.0) gv = dec-fabs(glon);
        if (gv > +180.0) gv -= 360.0;
        if (gv < -180.0) gv += 360.0;
        }
    otime = gtime;
    oalt = alt;
    olat = glat;
    olon = glon;
    return true;
}