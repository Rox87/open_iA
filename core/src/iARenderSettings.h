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

//! Settings for vtkRenderer, and helpers defined in iARenderer.
class iARenderSettings
{
public:
	bool
		ShowSlicers,			//! TODO: VOLUME: move to iAVolumeSettings?
		ShowSlicePlanes,
		ShowHelpers,
		ShowRPosition,
		ParallelProjection,
		UseFXAA,
		UseDepthPeeling;
	QString BackgroundTop,
		BackgroundBottom;
	float PlaneOpacity;
	int DepthPeels;

	iARenderSettings() :
		ShowSlicers(false),
		ShowSlicePlanes(false),
		ShowHelpers(true),
		ShowRPosition(true),
		ParallelProjection(false),
		UseFXAA(true),
		UseDepthPeeling(true),
		BackgroundTop("#7FAAFF"),
		BackgroundBottom("#FFFFFF"),
		PlaneOpacity(0.8f),
		DepthPeels(4)
	{}
};
