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
#pragma once

#include "open_iA_Core_export.h"

#include "iAValueType.h"

#include <cstddef> // for size_t
#include <cmath>   // for log

//! Abstract base class providing data used for drawing a plot in a chart.
class open_iA_Core_API iAPlotData
{
public:
	typedef double DataType;
	virtual ~iAPlotData() {}
	virtual DataType const * rawData() const =0;
	virtual size_t numBin() const =0;
	virtual double minX() const { return 0; }
	virtual double maxX() const { return static_cast<double>(numBin()); }
	virtual double spacing() const = 0;
	virtual double const * xBounds() const = 0;
	virtual DataType const * yBounds() const = 0;

	virtual double binStart(size_t binNr) const		// default: assume constant (i.e. linear) spacing
	{
		return spacing() * binNr + xBounds()[0];
	}
	virtual iAValueType valueType() const
	{
		return Continuous;
	}
};
