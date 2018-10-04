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

#include "iAProgress.h"

#include <QSharedPointer>
#include <QThread>

#include <vector>

class iAFiberResultsCollection;

class iARefDistCompute : public QThread
{
	Q_OBJECT
public:
	static const int DistanceMetricCount = 6;
	static const int BestDistanceMetric = 4;
	static const int EndColumns = 2;
	static int MaxNumberOfCloseFibers;
	iARefDistCompute(QSharedPointer<iAFiberResultsCollection> data, int referenceID);
	void run() override;
	iAProgress* progress();
	size_t referenceID() const;
private:
	iAProgress m_progress;
	QSharedPointer<iAFiberResultsCollection> m_data;
	int m_referenceID;
};
