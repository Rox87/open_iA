/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2018  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan,            *
*                          J. Weissenböck, Artem & Alexander Amirkhanov, B. Fröhler   *
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

#include <cstddef>    // for size_t
#include <vector>

class QColor;
class vtkLookupTable;

//! Class representing lookup table for color coding used in scatter plot matrix (SPLOM).
/*!
	Has methods for importing existing VTK lookup table (!if VTK is enabled via a preprocessor flag!)
	and mapping scalar values to a corresponding QColor.
*/
class open_iA_Core_API iALookupTable
{
public:
	//! Set up an empty (uninitialized) iALookupTable
	iALookupTable();

	//! Initialize an iALookupTable from the given vtkLookupTable
	explicit iALookupTable(vtkLookupTable * vtk_lut);

	//!  Copies data from the existing VTK lookup table (vtkLookupTable)
	void copyFromVTK(vtkLookupTable * vtk_lut);

	//!  Map a scalar value into an RGBA color.
	void getColor(double val, double * rgba_out);

	//!  Allocate place for a given number of colors and fill with zeros.
	void allocate(size_t numberOfColors);

	//!  Assign an RGBA color to a given index in the table.
	void setColor(size_t colInd, double * rgba);

	//!  Assign an QColor color to a given index in the table.
	void setColor(size_t colInd, QColor const & col);

	//! Fill the lookup table using provided raw RBGA data for a given number of colors.
	void setData(size_t numberOfColors, double * rgba_data);

	//! Set a given alpha value for every color in the table
	void setOpacity(double alpha);

	/* Setters/Getters */
	const double * getRange() const;                //!< Get the mapped scalar range.
	void setRange(double from_val, double to_val);  //!< Set the mapped scalar range.
	void setRange(double const * range);            //!< Set the mapped scalar range.
	bool initialized() const;                       //!< Check if the table has data (initialized).

protected:
	bool m_isInitialized;                   //!< flag which is on if lookup table data is set
	std::vector<double> m_data;             //!< lookup table raw color data, each color is 4 doubles (RGBA)
	double m_range[2];                      //!< scalar range mapped by the lookup table
	double m_rangeLen;                      //!< length of the total scalar range that is mapped by the lookup table
	size_t m_numColors;                     //!< number of colors stored in the lookup table
};
