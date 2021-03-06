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

#include "iAFeatureTrackingCorrespondence.h"

#include <vtkSmartPointer.h>

#include <QString>

#include <string>
#include <vector>

class vtkTable;
class vtkVariantArray;

class iAFeatureTracking
{
private:
	QString file1, file2, outputFilename;
	int lineOffset;
	vtkSmartPointer<vtkTable> u, v;
	float dissipationThreshold;
	float overlapThreshold;
	float volumeThreshold;
	float overallMatchingPercentage;
	int m_maxSearchValue;
	std::vector<std::pair<vtkIdType, std::vector<iAFeatureTrackingCorrespondence> > > *uToV;
	std::vector<std::pair<vtkIdType, std::vector<iAFeatureTrackingCorrespondence> > > *vToU;
	std::vector<std::pair<vtkIdType, std::vector<iAFeatureTrackingCorrespondence> > > *allUtoV;
	std::vector<std::pair<vtkIdType, std::vector<iAFeatureTrackingCorrespondence> > > *allVtoU;
	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
	std::vector<std::string> split(const std::string &s, char delim);
	vtkSmartPointer<vtkTable> readTableFromFile(const QString &filename, int dataLineOffset);
	void sortCorrespondencesByOverlap(std::vector<iAFeatureTrackingCorrespondence> &correspondences);
	std::vector<iAFeatureTrackingCorrespondence>& getCorrespondences(const vtkVariantArray &row, vtkTable &table, int maxSearchValue, bool useZ);
	void ComputeOverallMatchingPercentage();

public:
	iAFeatureTracking(QString const & fileName1, QString const & fileName2, int lineOffset, QString const & outputFilename, float dissipationThreshold,
		float overlapThreshold, float volumeThreshold, int maxSearchValue);
	void TrackFeatures();
	std::vector<iAFeatureTrackingCorrespondence> FromUtoV(unsigned int uId);
	std::vector<iAFeatureTrackingCorrespondence> FromVtoU(unsigned int vId);
	float GetOverallMatchingPercentage();

	size_t getNumberOfEventsInU();
	size_t getNumberOfEventsInV();
	vtkSmartPointer<vtkTable> getU();
	vtkSmartPointer<vtkTable> getV();
};
