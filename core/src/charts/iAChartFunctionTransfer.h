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

#include "iAChartFunction.h"
#include "iATransferFunction.h"
#include "open_iA_Core_export.h"

#include <QLinearGradient>

class QColorDialog;
class QDomNode;

class vtkPiecewiseFunction;
class vtkColorTransferFunction;

class open_iA_Core_API iAChartTransferFunction : public iAChartFunction, public iATransferFunction
{
Q_OBJECT

public:
	iAChartTransferFunction(iAChartWithFunctionsWidget *histogram, QColor color);
	~iAChartTransferFunction();

	int getType() override { return TRANSFER; }
	void draw(QPainter &painter) override;
	void draw(QPainter &painter, QColor color, int lineWidth) override;
	void drawOnTop(QPainter &painter) override;
	int selectPoint(QMouseEvent *event, int *x = nullptr) override;
	int getSelectedPoint() override { return m_selectedPoint; }
	int addPoint(int x, int y) override;
	void addColorPoint(int x, double red = -1.0, double green = -1.0, double blue = -1.0) override;
	void removePoint(int index) override;
	void moveSelectedPoint(int x, int y) override;
	void changeColor(QMouseEvent *event) override;
	void mouseReleaseEventAfterNewPoint(QMouseEvent *event) override;
	bool isColored() override { return true; }
	bool isEndPoint(int index) override;
	bool isDeletable(int index) override;
	void reset() override;
	size_t numPoints() const override;

	vtkPiecewiseFunction* opacityTF() override { return m_opacityTF; }
	vtkColorTransferFunction* colorTF() override { return m_colorTF; }

	void TranslateToNewRange(double const oldDataRange[2]);
	void enableRangeSliderHandles( bool rangeSliderHandles );
	void setOpacityFunction(vtkPiecewiseFunction *opacityTF) { m_opacityTF = opacityTF; }
	void setColorFunction(vtkColorTransferFunction *colorTF) { m_colorTF = colorTF; }
	void triggerOnChange();
signals:
	void Changed();
private:
	void setColorPoint(int selectedPoint, double x, double red, double green, double blue);
	void setColorPoint(int selectedPoint, int x, double red, double green, double blue);
	void setColorPoint(int selectedPoint, int x);
	void setPoint(int selectedPoint, int x, int y);
	void setPointX(int selectedPoint, int x);
	void setPointY(int selectedPoint, int y);

	// convert view to data
	double v2dX(int x);
	double v2dY(int y);

	// conver data to view
	int d2vX(double x, double oldDataRange0 = -1, double oldDataRange1 = -1);
	int d2vY(double y);

	//convert data to image
	int d2iX(double x);
	int d2iY(double y);

	bool m_rangeSliderHandles;
	int m_selectedPoint;

	QColor          m_color;
	QColorDialog    *m_colorDlg;
	QLinearGradient m_gradient;

	vtkPiecewiseFunction     *m_opacityTF;
	vtkColorTransferFunction *m_colorTF;

	static const int PIE_RADIUS = 16;
	static const int PIE_SIZE = 2 * PIE_RADIUS;

	static const int SELECTED_PIE_RADIUS = 16;
	static const int SELECTED_PIE_SIZE = 2 * SELECTED_PIE_RADIUS;
};
