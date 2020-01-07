#include "ThresholdCalcHelper.h"
#include <numeric>
#include "ThresholdDefinitions.h"
#include <algorithm>
#include <stdexcept>

#include "iAConsole.h"
#include "QLine"
#include "iAMathUtility.h"

#include <cmath>

namespace algorithm
{
	bool greaterThan(double u, double v)
	{
		return u > v;
	}

	bool smallerThan(double u, double v) {
		return u < v;
	}

	const double epsilon = 0.0000000001;

	bool compareDouble(double a, double b)
	{
		return fabs(a - b) < epsilon;
	}

	bool compareDouble(float a, float b)
	{
		return fabs(a - b) < (float)epsilon;
	}

	//checks whether a double is within a certian range
	bool compareDouble(double a, double b, double toleranceVal)
	{
		return fabs(a - b) < toleranceVal;
	}

	bool compareDoube(float a, float b, float toleranceVal)
	{
		return fabs(a - b) < toleranceVal;
	}


	bool isInfNegativeInf(float a)
	{
		return a == -INFINITY;
	}

	bool isInfNegativeInf(double a)
	{
		return a == -INFINITY;
	}


	/*bool validatePoints(const QPointF& pts)
	{
		return  (isInfNegativeInf(pts.x()) )
	}*/

}

double ThresholdCalcHelper::findMaxPeak(std::vector<double>& v_ind) const
{
	std::sort(v_ind.begin(), v_ind.end(), algorithm::greaterThan);
	auto peak = std::adjacent_find(v_ind.begin(), v_ind.end(), std::greater<double>());

	if (peak == v_ind.end()) {
		--peak;
	}

	return *peak;
}

double ThresholdCalcHelper::findMinPeak(std::vector<double>& v_ind) const {
	std::sort(v_ind.begin(), v_ind.end(), algorithm::smallerThan);
	auto peak = std::adjacent_find(v_ind.begin(), v_ind.end(), std::less<double>());

	if (peak == v_ind.end()) {
		--peak;
	}

	return *peak;
}

double ThresholdCalcHelper::vectorSum(const std::vector<double>& vec, size_t startInd, size_t endInd)
{
	if (startInd >= vec.size() || endInd >= vec.size()) throw new std::invalid_argument("test");
	double tmp = 0.0f;

	size_t i = startInd;
	while (i <= endInd) {

		tmp += vec[i];
		++i;
	}

	return tmp;
}


threshold_defs::ThresIndx ThresholdCalcHelper::findIndex(const std::vector<double>& vec, double cmpVal) const
{

	threshold_defs::ThresIndx thrInd;
	long ind = 0;
	thrInd.value = cmpVal;
	if (vec.empty()) {
		thrInd.thrIndx = -1; 
	}

	for (const double& el : vec) {
		bool isEqual = algorithm::compareDouble(el, cmpVal);
		if (isEqual) {
			thrInd.thrIndx = ind;
			break;
		}
		else {
			thrInd.thrIndx = -1;
		}

		ind++;
	}

	return thrInd;

}

threshold_defs::ThresMinMax ThresholdCalcHelper::calculateLocalePeaks(const threshold_defs::ParametersRanges& inRanges) const
{
	double y_min = 0;
	double y_max = 0;
	std::vector<double> yRange = inRanges.getYRange();
	std::vector<double> xRange = inRanges.getXRange();
	
	//or copy this because the array is sorted
	y_max = this->findMaxPeak(yRange);
	y_min = this->findMinPeak(yRange);
	DEBUG_LOG(QString("peaks at y %1 %2").arg(y_max).arg(y_min));
	
	const auto indMax = this->findIndex(inRanges.getYRange(), y_max);
	const auto indMin = this->findIndex(inRanges.getYRange(), y_min);
	
	if ((indMin.thrIndx == -1) || (indMax.thrIndx == -1)) 
		throw std::invalid_argument("index out of range");
	
	double x_max = xRange[indMax.thrIndx];
	double x_min = xRange[indMin.thrIndx];	
	
	threshold_defs::ThresMinMax thrMinMax;
	
	thrMinMax.FreqPeakLokalMaxY(y_max);
	thrMinMax.FreqPeakMinY(y_min);
	thrMinMax.PeakMinXThreshold(x_min);
	thrMinMax.LokalMaxPeakThreshold_X(x_max);
	return thrMinMax; 
}

void ThresholdCalcHelper::determinIso50andGlobalMax(const threshold_defs::ParametersRanges& inRanges, threshold_defs::ThresMinMax &inVals)
{
	//min_Max of a Range
	//detect peak max
	//iso 50 is between air and material peak - grauwert

	try{
			
		std::vector<double> freqRangesY = inRanges.getYRange();
		double maxRange = this->findMaxPeak(freqRangesY); //maximum 
		
		threshold_defs::ThresIndx indMinMax = findIndex(inRanges.getYRange(), maxRange);
		if (indMinMax.thrIndx < 0) {
			return; 
		}
		
		double maxPeakThres = inRanges.getXRange()[indMinMax.thrIndx]; 
		double Iso50Val = (maxPeakThres + inVals.LokalMaxPeakThreshold_X()) * 0.5f;
		inVals.setMaterialsThreshold(maxPeakThres); 				
		inVals.Iso50ValueThr(Iso50Val);
	}
	catch (std::invalid_argument& /*iae*/) {
		throw; 
	}

}


void ThresholdCalcHelper::getFirstElemInRange(const QVector <QPointF>& in, float xmin, float xmax, QPointF* result)
{
	if (!result) throw std::invalid_argument("null argument QPointF"); 
	if (in.empty())
	{ 
		result = nullptr;
		return;
	}

	
	try 
	{
	
		QVector<QPointF> ranges = in;
		sortPointsByX(ranges);
		bool contained = false;
		QPointF pt_tmp;

		for (const QPointF& pt : ranges)
		{
			if (this->checkInRange(pt, xmin, xmax))
			{
				contained = true;
				pt_tmp = pt;
				break;
			}
		}

		if (contained)
		{
			result->setX(pt_tmp.x());
			result->setY(pt_tmp.y());

		}
		else
		{
			result = nullptr;
		}
	}
	catch (std::invalid_argument& /*ia*/) {
		throw; 
	}
	catch (std::bad_alloc& /*ba*/) {
		DEBUG_LOG("error calculation elem by ranges faild in memory");
		throw; 
	}
	
}

void ThresholdCalcHelper::PeakgreyThresholdNormalization(threshold_defs::ParametersRanges& ranges, double greyThrPeakAir, double greyThrPeakMax)
{
	if (greyThrPeakAir < std::numeric_limits<double>::min() || (greyThrPeakAir > std::numeric_limits<double>::max())) {
		DEBUG_LOG(QString("grey value threshold invalid %1").arg(greyThrPeakAir));
		return; 
	}

	if (greyThrPeakAir > greyThrPeakMax) {
		DEBUG_LOG("grey value of air peak must be smaller than matierial peak, please change order");
		return; 
	}


	std::vector<double> tmp_ranges_x = ranges.getXRange();
	if (tmp_ranges_x.empty()) {
		return; 		
	}


	for (double& val: tmp_ranges_x) {
		DEBUG_LOG(QString("before %1").arg(val));
		val = minMaxNormalize(greyThrPeakAir, greyThrPeakMax, val);
		DEBUG_LOG(QString("after %1").arg(val))
	}

	ranges.setXVals(tmp_ranges_x);
	
}

bool ThresholdCalcHelper::checkInRange(const QPointF& pt, float min, float max)
{
	float xval = (float)pt.x();
	bool isInRangeMin = false;
	bool isInRangeMax = false;

	if (!(min < max))
	{
		throw std::invalid_argument("error comparing values");
	}

	isInRangeMin = (algorithm::compareDouble(min, xval)) || (min < xval);
	isInRangeMax = (algorithm::compareDouble(max, xval)) || (xval < max);

	return (isInRangeMax && isInRangeMin);

}