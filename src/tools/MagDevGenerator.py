import numpy as np 

def toSN(num):
    string='%.6e'%num
    if string[0] != '-':
        string = ' '+string
    return string


def writearray(output, name, arr):
    if len(arr.shape) == 1:
        n = arr.shape[0]
        output.write('double %s[%d] = { '%(name,n))
        for i in range(n):
            output.write(toSN(arr[i]))
            if i < n-1:
                output.write(', ')
        output.write('};\n')
    if len(arr.shape) == 2:
        n = arr.shape[0]
        m = arr.shape[1]
        output.write('double %s[%d][%d] = {\n'%(name,n,m))
        for i in range(n):
            output.write('   { ')
            for j in range(m):
                output.write(toSN(arr[i,j]))
                if j < m-1:
                    output.write(', ')
            output.write('}')
            if i < n-1:
                output.write(',')
            output.write('\n')
        output.write('};\n')


#static int maxord,i,icomp,n,m,j,D1,D2,D3,D4;

#static double c[13][13],cd[13][13],tc[13][13],dp[13][13],snorm[169],
    # sp[13],cp[13],fn[13],fm[13],pp[13],k[13][13],pi,dtr,a,b,re,
    # a2,b2,c2,a4,b4,c4,epoch,gnm,hnm,dgnm,dhnm,flnmj,otime,oalt,
    # olat,olon,dt,rlon,rlat,srlon,srlat,crlon,crlat,srlat2,
    # crlat2,q,q1,q2,ct,st,r2,r,d,ca,sa,aor,ar,br,bt,bp,bpp,
    # par,temp1,temp2,parp,bx,by,bz,bh;
c = np.zeros([13,13])
cd = np.zeros([13,13])
tc = np.zeros([13,13])
dp = np.zeros([13,13])
snorm = np.zeros(169)
sp = np.zeros(13)
cp = np.zeros(13)
fn = np.zeros(13)
fm = np.zeros(13)
pp = np.zeros(13)
k = np.zeros([13,13])

sp[0] = 0.0
cp[0] = 1.0
pp[0] = 1.0
#p = snorm pointer
dp[0][0] = 0.0
a = 6378.137
b = 6356.7523142
re = 6371.2
a2 = a*a
b2 = b*b
c2 = a2-b2
a4 = a2*a2
b4 = b2*b2
c4 = a4 - b4

# READ WORLD MAGNETIC MODEL SPHERICAL HARMONIC COEFFICIENTS
c[0][0] = 0.0
cd[0][0] = 0.0
  
# start file read
cof_file = open("WMM2015COF/WMM.COF")

# read header
##// %YYYY %s (epoch, model name)
epoch, model = cof_file.readline().split()[0:2]
epoch = float(epoch)
print("Epoch: [%d-%d] Model: %s"%(epoch, epoch+5, model))
frmt = [int, int, float, float, float, float]

for line in cof_file:
    if line[0:4] == "9999":
        break
    n,m,gnm,hnm,dgnm,dhnm = [f(s) for (f,s) in zip(frmt,line.split())]

    if (m > n or m < 0.0) :
        print("Corrupt record in model file WMM.COF")
        exit()

    c[m][n] = gnm
    cd[m][n] = dgnm
    if (m != 0):
        c[n][m-1] = hnm
        cd[n][m-1] = dhnm

# CONVERT SCHMIDT NORMALIZED GAUSS COEFFICIENTS TO UNNORMALIZED */
snorm[0] = 1.0
fm[0] = 0.0

for n in range(1,12+1) :
    snorm[n] = snorm[n-1]*float(2*n-1)/float(n)
    j = 2
    for m in range(0,n+1):
        print("n:%d\tm:%d"%(n,m))
        k[m][n] = float(((n-1)*(n-1))-(m*m))/float((2*n-1)*(2*n-3))
        if m > 0:
            flnmj = float((n-m+1)*j)/float(n+m)
            snorm[n+m*13] = snorm[n+(m-1)*13]*np.sqrt(flnmj)
            j = 1
            c[n][m-1] = snorm[n+m*13]*c[n][m-1]
            cd[n][m-1] = snorm[n+m*13]*cd[n][m-1]
        c[m][n] = snorm[n+m*13]*c[m][n]
        cd[m][n] = snorm[n+m*13]*cd[m][n]
    fn[n] = float(n+1)
    fm[n] = float(n)
k[1][1] = 0.0

output = open("WMM2015cof.h","w")
output.write("#pragma once\n")
output.write('char const WMMmodel[] = "%s";\n'%model)
output.write("double const epoch=%f;\n"%epoch)
writearray(output, 'const c', c)
writearray(output, 'const cd', cd)
writearray(output, 'const k', k)
writearray(output, 'const fn', fn)
writearray(output, 'const fm', fm)
writearray(output, 'snorm', snorm)