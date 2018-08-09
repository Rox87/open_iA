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

#include <QObject>

#include <vector>

#include <cstddef>    // for size_t

class iAQSplom;

class vtkTable;

class QColor;
class QDockWidget;

class iAFeatureScoutSPLOM: public QObject
{
	Q_OBJECT
public:
	iAFeatureScoutSPLOM();
	~iAFeatureScoutSPLOM();
	void initScatterPlot(QDockWidget* container, vtkTable* csvTable);  //!< initialize SPLOM and show in given container
	void updateColumnVisibility(std::vector<bool> columnVisibility);   //!< update column visibility
	void setDotColor(QColor const & color, double const range[2]);     //!< set color for all SPLOM dots (TODO: move range calculations to iASplomData!)
	void setFilter(int classID);                                       //!< specify a filter on class column
	void multiClassRendering(QList<QColor> const & colors);            //!< colors each dot according to its class
	void setSelection(std::vector<size_t> selection);                  //!< set selection in SPLOM
	void classAdded(int classID);                                      //!< notifies SPLOM that class was added of current selection
	void classDeleted(int classID);                                    //!< notifies SPLOM that class was deleted
	void changeClass(size_t objID, int classID);                       //!< set class of single object to given ID
	void classesChanged();
	std::vector<size_t> getFilteredSelection() const;                  //!< proxy for getFilteredSelection in SPLOM
	bool isShown() const;
	void clearSelection();
signals:
	void selectionModified(std::vector<size_t>);
private:
	iAQSplom * matrix;
};