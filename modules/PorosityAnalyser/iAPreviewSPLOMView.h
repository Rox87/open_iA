/*********************************  open_iA 2016 06  ******************************** *
* **********  A tool for scientific visualisation and 3D image processing  ********** *
* *********************************************************************************** *
* Copyright (C) 2016  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, J. Weissenb�ck, *
*                     Artem & Alexander Amirkhanov, B. Fr�hler                        *
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
* Contact: FH O� Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstra�e 23, 4600 Wels / Austria, Email:                           *
* ************************************************************************************/
 
#pragma once

#include <QDockWidget>
#include <QScopedPointer>

#include "ui_PreviewSPLOM.h"
#include "iAQTtoUIConnector.h"

class iAPreviewSPLOM;
class QPixmap;

typedef iAQTtoUIConnector<QDockWidget, Ui_PreviewSPLOM>  PreviewSPLOMConnector;

class iAPreviewSPLOMView : public PreviewSPLOMConnector
{
	Q_OBJECT
public:
	iAPreviewSPLOMView( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	void SetDatasetsDir( const QString & datasetsFolder );
	~iAPreviewSPLOMView();

public slots:
	void SetDatasets( QStringList datasets );
	void SliceNumberChanged();
	void SetSliceCount( int sliceCnt );
	void LoadDatasets();
	void clear();
	void SetMask( const QPixmap * mask, int datasetIndex = -1 );

protected slots:
	void ROIChangedSlot( QRectF roi );
	void UpdatePixmap();
	void DatasetChanged();
	void SetSlice( int slice );

signals:
	void roiChanged( QList<QRectF> roi );
	void sliceCountsChanged( QList<int> sliceCntLst );
	void sliceNumbersChanged( QList<int> sliceNumberLst );

protected:
	QString m_datasetsDir;
	QStringList m_datasets;
	QList<int> m_sliceNumberLst;
	QList<int> m_sliceCntLst;
	iAPreviewSPLOM * m_preview;
	QScopedPointer<QPixmap> m_slicePxmp;
	QList<QRectF> m_roiList;
	bool m_datasetsLoaded;
};