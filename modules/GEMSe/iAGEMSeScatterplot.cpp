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
*          Stelzhamerstra�e 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#include "pch.h"
#include "iAGEMSeScatterplot.h"

#include "iAAttributes.h"
#include "iAAttributeDescriptor.h"
#include "iAChartAttributeMapper.h"
#include "iAChartFilter.h"
#include "iAImageTreeLeaf.h"
#include "iAImageTreeNode.h"

#include <vtkAxis.h>
#include <vtkFloatArray.h>
#include <vtkChartXY.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPlot.h>
#include <vtkTable.h>

#include <QLayout>
#include <QWidget>

iAGEMSeScatterplot::iAGEMSeScatterplot(QWidget* parent):
	m_singlePlot(0),
	m_clusterPlot(0),
	m_parent(parent)
{
	vtkSmartPointer<vtkContextView> contextView(vtkSmartPointer<vtkContextView>::New());
	contextView->SetRenderWindow(GetRenderWindow());
	m_chart = vtkSmartPointer<vtkChartXY>::New();
	m_chart->SetSelectionMode(vtkContextScene::SELECTION_NONE);
	contextView->GetScene()->AddItem(m_chart);

	m_parent->layout()->addWidget(this);
	m_parent->hide();
}


void AddTableValues(
	vtkSmartPointer<vtkTable> table,
	iAImageTreeNode const * node, int p1, int p2, int & rowNr,
	iAChartFilter const & attribFilter,
	iAChartAttributeMapper const & chartAttrMap)
{
	if (node->IsLeaf())
	{
		iAImageTreeLeaf const * leaf = dynamic_cast<iAImageTreeLeaf const *>(node);
		if (attribFilter.Matches(leaf, chartAttrMap) &&
			chartAttrMap.GetDatasetIDs(p1).contains(leaf->GetDatasetID()) &&
			chartAttrMap.GetDatasetIDs(p2).contains(leaf->GetDatasetID()))
		{
			int attr1Idx = chartAttrMap.GetAttributeID(p1, leaf->GetDatasetID());
			int attr2Idx = chartAttrMap.GetAttributeID(p2, leaf->GetDatasetID());
			double attr1Value = node->GetAttribute(attr1Idx);
			double attr2Value = node->GetAttribute(attr2Idx);
			table->SetValue(rowNr, 0, attr1Value);
			table->SetValue(rowNr, 1, attr2Value);
			rowNr++;
		}
	}
	else
	{
		for (int i = 0; i<node->GetChildCount(); ++i)
		{
			AddTableValues(table, node->GetChild(i).data(), p1, p2, rowNr, attribFilter, chartAttrMap);
		}
	}
}


vtkSmartPointer<vtkTable> iAGEMSeScatterplot::GetComparisonTable(
	iAImageTreeNode const * node,
	iAChartFilter const & attribFilter,
	iAChartAttributeMapper const & chartAttributeMapper)
{
	if (!node)
	{
		return vtkSmartPointer<vtkTable>();
	}
	vtkSmartPointer<vtkTable> comparisonTable = vtkSmartPointer<vtkTable>::New();

	int numberOfSamples = node->GetClusterSize();
	vtkSmartPointer<vtkFloatArray> arrX(vtkSmartPointer<vtkFloatArray>::New());
	vtkSmartPointer<vtkFloatArray> arrY(vtkSmartPointer<vtkFloatArray>::New());
	arrX->SetName(m_chart1Name.toStdString().c_str());
	arrY->SetName(m_chart2Name.toStdString().c_str());
	comparisonTable->AddColumn(arrX);
	comparisonTable->AddColumn(arrY);
	// we may call SetNumberOfRows only after we've added all columns, otherwise there is an error
	comparisonTable->SetNumberOfRows(numberOfSamples);

	int rowNr = 0;
	AddTableValues(comparisonTable, node, m_chart1ID, m_chart2ID, rowNr, attribFilter,
		chartAttributeMapper);

	// reduce number of rows to actual number:
	comparisonTable->SetNumberOfRows(rowNr);
	return comparisonTable;
}


void iAGEMSeScatterplot::ChangeDataSource(
	int chart1ID, int chart2ID,
	QString const & chart1Name, QString const & chart2Name,
	bool chart1Log, bool chart2Log,
	iAChartAttributeMapper const & chartAttributeMapper,
	iAChartFilter const & chartFilter,
	iAImageTreeNode const * root,
	iAImageTreeNode const * cluster,
	iAImageTreeNode const * leaf)
{
	m_chart1ID = chart1ID;
	m_chart2ID = chart2ID;
	m_chart1Name = chart1Name;
	m_chart2Name = chart2Name;
	m_chart1Log = chart1Log;
	m_chart2Log = chart2Log;
	iAChartFilter emptyFilter;
	// TODO do we need m_comparisonTable as member?
	m_allTable = GetComparisonTable(root, emptyFilter,
		chartAttributeMapper);
	if (!m_allTable)
	{
		m_parent->hide();
		return;
	}
	m_chart->ClearPlots();
	m_singlePlot = 0;
	m_clusterPlot = 0;
	auto xAxis = m_chart->GetAxis(vtkAxis::BOTTOM);
	auto yAxis = m_chart->GetAxis(vtkAxis::LEFT);
	xAxis->SetTitle(m_chart1Name.toStdString());
	xAxis->SetLogScale(m_chart1Log);
	yAxis->SetTitle(m_chart2Name.toStdString());
	yAxis->SetLogScale(m_chart2Log);

	vtkPlot* plot = m_chart->AddPlot(vtkChart::POINTS);
	plot->SetColor(
		static_cast<unsigned char>(DefaultColors::AllDataChartColor.red()),
		static_cast<unsigned char>(DefaultColors::AllDataChartColor.green()),
		static_cast<unsigned char>(DefaultColors::AllDataChartColor.blue()),
		static_cast<unsigned char>(DefaultColors::AllDataChartColor.alpha())
	);
	plot->SetWidth(1.0);
	plot->SetInputData(m_allTable, 0, 1);
	UpdateClusterPlot(cluster, chartFilter, chartAttributeMapper);
	UpdateLeafPlot(leaf, chartAttributeMapper);
	if (!chartFilter.MatchesAll())
	{
		Update(
			m_allFilteredTable, m_allFilteredPlot, root,
			DefaultColors::FilteredChartColor, chartFilter,
			chartAttributeMapper);
	}
}


void iAGEMSeScatterplot::Update(
	vtkSmartPointer<vtkTable> & table,
	vtkSmartPointer<vtkPlot> & plot,
	iAImageTreeNode const * node,
	QColor const & color,
	iAChartFilter const & filter,
	iAChartAttributeMapper const & chartAttributeMapper)
{
	if (plot)
	{
		m_chart->RemovePlotInstance(plot);
		plot = 0;
	}
	if (!node)
	{
		return;
	}
	table = GetComparisonTable(node, filter, chartAttributeMapper);
	if (!table)
	{
		return;
	}
	plot = m_chart->AddPlot(vtkChart::POINTS);
	// TODO: set plot order? at the moment this is done only implicitly by callingt AddPlot in the right order

	plot->SetColor(
		static_cast<unsigned char>(color.red()),
		static_cast<unsigned char>(color.green()),
		static_cast<unsigned char>(color.blue()),
		static_cast<unsigned char>(color.alpha())
	);
	plot->SetInputData(table, 0, 1);
	m_parent->show();
}

void iAGEMSeScatterplot::UpdateFilteredAllPlot(
	iAImageTreeNode const * root, iAChartFilter const & chartFilter,
	iAChartAttributeMapper const & chartAttributeMapper)
{
	if (!chartFilter.MatchesAll())
	{
		Update(
			m_allFilteredTable,
			m_allFilteredPlot,
			root,
			DefaultColors::FilteredChartColor,
			chartFilter,
			chartAttributeMapper
		);
	}
}


void iAGEMSeScatterplot::UpdateLeafPlot(
	iAImageTreeNode const * selectedLeaf,
	iAChartAttributeMapper const & chartAttributeMapper)
{
	iAChartFilter emptyFilter;
	Update(m_singleTable, m_singlePlot, selectedLeaf, DefaultColors::ImageChartColor, emptyFilter,
		chartAttributeMapper);
}


void iAGEMSeScatterplot::UpdateClusterPlot(
	iAImageTreeNode const * selectedCluster, iAChartFilter const & chartFilter,
	iAChartAttributeMapper const & chartAttributeMapper)
{
	iAChartFilter emptyFilter;
	Update(
		m_clusterTable,
		m_clusterPlot,
		selectedCluster,
		DefaultColors::ClusterChartColor[0],
		emptyFilter,
		chartAttributeMapper
	);
	if (!chartFilter.MatchesAll())
	{
		Update(
			m_clusterFilteredTable,
			m_clusterFilteredPlot,
			selectedCluster,
			DefaultColors::FilteredClusterChartColor,
			chartFilter,
			chartAttributeMapper
		);
	}
}
