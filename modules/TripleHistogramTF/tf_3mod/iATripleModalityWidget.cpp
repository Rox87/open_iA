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

#include "iATripleModalityWidget.h"

#include "iAHistogramAbstract.h"
#include "iABarycentricContextRenderer.h"
#include "iABarycentricTriangleWidget.h"
#include "iASimpleSlicerWidget.h"
#include "RightBorderLayout.h"

#include <charts/iAChartWithFunctionsWidget.h>
#include <iAModality.h>

#include <QComboBox>
#include <QSharedPointer>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>

iATripleModalityWidget::iATripleModalityWidget(QWidget * parent, MdiChild *mdiChild, Qt::WindowFlags f /*= 0 */) :
	iAMultimodalWidget(parent, mdiChild, THREE)
{
	m_triangleRenderer = new iABarycentricContextRenderer();
	m_triangleWidget = new iABarycentricTriangleWidget();

	m_triangleWidget->setTriangleRenderer(m_triangleRenderer);
	m_layoutComboBox = new QComboBox();
	m_layoutComboBox->addItem("Stack", iAHistogramAbstractType::STACK);
	m_layoutComboBox->addItem("Triangle", iAHistogramAbstractType::TRIANGLE);

	// Initialize the inner widget
	setHistogramAbstractType(iAHistogramAbstractType::STACK);

	connect(m_layoutComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(layoutComboBoxIndexChanged(int)));
	connect(m_triangleWidget, SIGNAL(weightsChanged(BCoord)), this, SLOT(triangleWeightChanged(BCoord)));
	connect(m_triangleWidget, SIGNAL(weightsChanged(BCoord)), this, SLOT(weightsChangedSlot(BCoord)));

	connect(this, SIGNAL(modalitiesLoaded_beforeUpdate()), this, SLOT(modalitiesLoaded_beforeUpdateSlot()));

	if (isReady()) {
		updateModalities();
	}
}

iATripleModalityWidget::~iATripleModalityWidget()
{
	delete m_triangleRenderer;
}

void iATripleModalityWidget::layoutComboBoxIndexChanged(int newIndex)
{
	setLayoutTypePrivate(getLayoutTypeAt(newIndex));
}

iAHistogramAbstractType iATripleModalityWidget::getLayoutTypeAt(int comboBoxIndex) {
	return (iAHistogramAbstractType)m_layoutComboBox->itemData(comboBoxIndex).toInt();
}

void iATripleModalityWidget::updateModalities()
{
	QString names[3];
	for (int i = 0; i < 3; ++i)
		names[i] = getModality(i)->name();
	m_triangleWidget->setModalities(getModality(0)->image(), getModality(1)->image(), getModality(2)->image());
	m_triangleWidget->updateModalityNames(names);
	m_triangleWidget->update();
}

void iATripleModalityWidget::modalitiesChanged()
{
	QString names[3];
	for (int i = 0; i < 3; ++i)
		names[i] = getModality(i)->name();
	m_triangleWidget->updateModalityNames(names);
	m_histogramAbstract->updateModalityNames(names);
}

// SLOT
void iATripleModalityWidget::triangleWeightChanged(BCoord newWeight)
{
	setWeightsProtected(newWeight);
}

// SLOT
void iATripleModalityWidget::weightsChangedSlot(BCoord bCoord)
{
	if (bCoord != getWeights()) {
		m_triangleWidget->setWeight(bCoord);
	}
}

// SLOT
void iATripleModalityWidget::modalitiesLoaded_beforeUpdateSlot() {
	updateModalities();
	QString names[3];
	for (int i = 0; i < 3; ++i)
		names[i] = getModality(i)->name();
	m_histogramAbstract->initialize(names);
}

void iATripleModalityWidget::setHistogramAbstractType(iAHistogramAbstractType type) {
	setLayoutTypePrivate(type);
	m_layoutComboBox->setCurrentIndex(m_layoutComboBox->findData(type));
}

void iATripleModalityWidget::setLayoutTypePrivate(iAHistogramAbstractType type) {
	if (m_histogramAbstract && type == m_histogramAbstractType) {
		return;
	}

	iAHistogramAbstract *histogramAbstract_new = iAHistogramAbstract::buildHistogramAbstract(type, this, m_mdiChild);

	if (m_histogramAbstract) {
		for (int i = 0; i < 3; i++) {
			w_histogram(i)->setParent(nullptr);
			w_slicer(i)->setParent(nullptr);
			resetSlicer(i);
		}
		m_triangleWidget->setParent(nullptr);
		m_layoutComboBox->setParent(nullptr);
		w_slicerModeLabel()->setParent(nullptr);
		w_sliceNumberLabel()->setParent(nullptr);
		w_checkBox_weightByOpacity()->setParent(nullptr);
		w_checkBox_syncedCamera()->setParent(nullptr);

		//delete m_histogramAbstract;
		m_innerLayout->replaceWidget(m_histogramAbstract, histogramAbstract_new, Qt::FindDirectChildrenOnly);

		delete m_histogramAbstract;
	} else {
		m_innerLayout->addWidget(histogramAbstract_new);
	}
	m_histogramAbstract = histogramAbstract_new;

	if (isReady()) {
		QString names[3];
		for (int i = 0; i < 3; ++i)
			names[i] = getModality(i)->name();
		m_histogramAbstract->initialize(names);
	}
}