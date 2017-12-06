#pragma once
class IIR_filter
{
public:
	IIR_filter(double coef);
	virtual ~IIR_filter();
	double do_sample(double data);
	bool setCoef(double);
	double getCoef(void);
	double lastValue;

protected:
	double coef;
};

