#include "stdafx.h"
#include "IIR_filter.h"
#include <stdexcept>

IIR_filter::IIR_filter(double coef)
{
	setCoef(coef);
	lastValue = 0;
}

double IIR_filter::do_sample(double data) {
	double delta = data - lastValue;
	double answer = lastValue + delta * getCoef();
	lastValue = answer;
	return lastValue;
}

bool IIR_filter::setCoef(double c) { 
	if (c >= 0) {
		coef = c;
		return true;
	} else {
		coef = 0;
		throw std::invalid_argument("received negative value");
		return false;
	}

}

double IIR_filter::getCoef() {
	return coef;
}
IIR_filter::~IIR_filter()
{
}
