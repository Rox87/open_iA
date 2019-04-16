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
 
#include <QSplitter>
#include <QResizeEvent>

#include "iAHistogramStack.h"
#include "iATransferFunction.h"
#include "charts/iADiagramFctWidget.h"

iAHistogramStack::iAHistogramStack(QWidget* parent, iATripleModalityWidget *tripleModalityWidget, MdiChild *mdiChild, Qt::WindowFlags f)
	: m_tmw(tripleModalityWidget)
{
}

void iAHistogramStack::initialize()
{
	const QString modalityLabels[3] = { QString("A"), QString("B"), QString("C") };
	for (int i = 0; i < 3; i++) {
		//m_modalityLabels[i] = new QLabel(DEFAULT_MODALITY_LABELS[i]);
		m_modalityLabels[i] = new QLabel(modalityLabels[i]);
		m_modalityLabels[i]->setStyleSheet("font-weight: bold");
		m_modalityLabels[i]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	}

	m_tmw->m_triangleWidget->recalculatePositions();

	m_tmw->m_sliceSlider->setOrientation(Qt::Horizontal);
	m_tmw->m_sliceSlider->setInvertedAppearance(false);

	QWidget *optionsContainer = new QWidget();
	//optionsContainer->setStyleSheet("background-color:blue"); // test spacing/padding/margin
	optionsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QHBoxLayout *optionsContainerLayout = new QHBoxLayout(optionsContainer);
	optionsContainerLayout->addWidget(m_tmw->m_layoutComboBox);
	optionsContainerLayout->addWidget(m_tmw->m_slicerModeComboBox);
	optionsContainerLayout->addWidget(m_tmw->m_sliceSlider);

	m_grid = new iAHistogramStackGrid(this, m_tmw->m_histograms, m_tmw->m_slicerWidgets, m_modalityLabels);

	QWidget *leftWidget = new QWidget();
	QVBoxLayout *leftWidgetLayout = new QVBoxLayout(leftWidget);
	leftWidgetLayout->setSpacing(1);
	leftWidgetLayout->setMargin(0);
	leftWidgetLayout->addWidget(optionsContainer);
	leftWidgetLayout->addWidget(m_grid);

	m_splitter = new QSplitter(Qt::Horizontal);
	m_splitter->addWidget(leftWidget);
	m_splitter->addWidget(m_tmw->m_triangleWidget);
	m_splitter->setStretchFactor(0, 1);
	m_splitter->setStretchFactor(1, 0);

	QLayout *parentLayout = new QHBoxLayout(this); // TODO we don't need any layout here because QSplitter is the only widget... what to do?
	parentLayout->addWidget(m_splitter);

	m_grid->adjustStretch();
}

// GRID -------------------------------------------------------------------------------

iAHistogramStackGrid::iAHistogramStackGrid(QWidget *parent, iADiagramFctWidget *histograms[3], iASimpleSlicerWidget *slicers[3], QLabel *labels[3], Qt::WindowFlags f)
	: QWidget(parent, f)
{
	m_gridLayout = new QGridLayout(this);
	for (int i = 0; i < 3; i++) {
		m_gridLayout->addWidget(histograms[i], i, 0);
		m_gridLayout->addWidget(slicers[i], i, 1);
		m_gridLayout->addWidget(labels[i], i, 2);
	}
	m_gridLayout->setSpacing(m_spacing);
	m_gridLayout->setMargin(0);
}

void iAHistogramStackGrid::resizeEvent(QResizeEvent* event)
{
	adjustStretch(event->size().width());
}

void iAHistogramStackGrid::adjustStretch(int totalWidth)
{
	//QLayoutItem *item00 = m_gridLayout->itemAtPosition(0, 0);
	//int histogramHeight = item00->widget()->size().height();
	int histogramHeight = size().height() / 3 - 4*m_spacing;
	m_gridLayout->setColumnStretch(0, totalWidth - histogramHeight);
	m_gridLayout->setColumnStretch(1, histogramHeight);
}