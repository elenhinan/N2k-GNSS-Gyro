#pragma once
#include <Stream.h>
//
void MagneticVariationTest(Stream *output);
bool MagneticVariation(double alt, double glat, double glon, double gtime, double &dec, double &dip, double &ti, double &gv);