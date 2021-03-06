/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2020  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
*                          Amirkhanov, J. Weissenböck, B. Fröhler, M. Schiwarth       *
* *********************************************************************************** *
* This program is free software: you can redistribute it and/or modify it under the   *
* terms of the GNU General Public License as published by the Free Software           *
* Foundation, either version 3 of the License, or (at your option) any later version. *
*                                                                                     *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY     *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A     *
* PARTICULAR PURPOSE.  See the GNU General Public License for more details.           *
*                                                                                     *
* You should have received a copy of the GNU General Public License along with this   *
* program.  If not, see http://www.gnu.org/licenses/                                  *
* *********************************************************************************** *
* Contact: FH OÖ Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstraße 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#include "iAMappingDiagramData.h"

#include <algorithm>
#include <cassert>
#include <cmath>

// TODO: Check if this can be replaced with vtkMath::Round
#if (defined(_MSC_VER) && _MSC_VER < 1800)
static inline double Round(double val)
{
	return floor(val + 0.5);
}
#else
#define Round std::round
#endif

iAMappingDiagramData::iAMappingDiagramData(DataType const * data,
	size_t srcNumBin, double srcMinX, double srcMaxX,
	size_t targetNumBin, double targetMinX, double targetMaxX,
	DataType const maxValue):
	m_numBin(targetNumBin),
	m_data(new DataType[m_numBin])
{
	assert(srcNumBin > 1 && targetNumBin > 1);
	double srcSpacing = (srcMaxX - srcMinX) / (srcNumBin-1);
	m_spacing = (targetMaxX - targetMinX) / (targetNumBin-1);
	m_xBounds[0] = targetMinX;
	m_xBounds[1] = targetMaxX;
	m_yBounds[0] = 0;
	m_yBounds[1] = maxValue;
	// get scale factor from all source data
	DataType myMax = *std::max_element(data, data+srcNumBin);
	double scaleFactor = static_cast<double>(maxValue) / myMax;

	// map source data to target indices:
	for (size_t i=0; i<targetNumBin; ++i)
	{
		double sourceIdxDbl = ((i * m_spacing) + targetMinX - srcMinX) / srcSpacing ;
		int sourceIdx = static_cast<int>(Round(sourceIdxDbl));

		m_data[i] = (sourceIdx >= 0 && static_cast<size_t>(sourceIdx) < srcNumBin) ?
			static_cast<DataType>(data[sourceIdx] * scaleFactor) : 0;
	}
}

iAMappingDiagramData::~iAMappingDiagramData()
{
	delete [] m_data;
}

size_t iAMappingDiagramData::numBin() const
{
	return m_numBin;
}

iAMappingDiagramData::DataType const * iAMappingDiagramData::rawData() const
{
	return m_data;
}

double iAMappingDiagramData::spacing() const
{
	return m_spacing;
}

double const * iAMappingDiagramData::xBounds() const
{
	return m_xBounds;
}

iAPlotData::DataType const * iAMappingDiagramData::yBounds() const
{
	return m_yBounds;
}