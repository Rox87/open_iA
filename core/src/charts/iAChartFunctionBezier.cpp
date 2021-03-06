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
#include "iAChartFunctionBezier.h"

#include "charts/iAChartWithFunctionsWidget.h"
#include "iAMapper.h"
#include "iAMathUtility.h"

#include <vtkImageData.h>

#include <QPen>
#include <QPainter>
#include <QMouseEvent>

#include <cassert>

iAChartFunctionBezier::iAChartFunctionBezier(iAChartWithFunctionsWidget *chart, QColor &color, bool res):
	iAChartFunction(chart),
	m_color(color),
	m_selectedPoint(-1)
{
	controlDist = chart->xRange() / 8;
	if (res)
	{
		reset();
	}
}

void iAChartFunctionBezier::draw(QPainter &painter)
{
	draw(painter, m_color, 3);
}

void iAChartFunctionBezier::draw(QPainter &painter, QColor penColor, int lineWidth)
{
	bool functionActive = (chart->selectedFunction() == this);

	// draw line
	QPen pen = painter.pen();
	pen.setColor(penColor);
	pen.setWidth(lineWidth);

	painter.setPen(pen);

	int pointNumber = static_cast<int>(realPoints.size());
	for(int l = 0; l < pointNumber-1; l+=3)
	{
		double X1 = realPoints[l].x();
		double Y1 = realPoints[l].y();

		// Variable
		double a = 1.0;
		double b = 1.0 - a;

		for (int i = 0; i <= 50; i++)
		{
			// Get a point on the curve
			double X2 = realPoints[l].x()*a*a*a + realPoints[l + 1].x() * 3 * a*a*b + realPoints[l + 2].x() * 3 * a*b*b + realPoints[l + 3].x()*b*b*b;
			double Y2 = realPoints[l].y()*a*a*a + realPoints[l + 1].y() * 3 * a*a*b + realPoints[l + 2].y() * 3 * a*b*b + realPoints[l + 3].y()*b*b*b;

			// Draw the line from point to point (assuming OGL is set up properly)
			int x1, y1;
			x1 = d2iX(X1);
			y1 = d2iY(Y1);

			int x2, y2;
			x2 = d2iX(X2);
			y2 = d2iY(Y2);

			painter.drawLine(x1, y1, x2, y2);

			X1 = X2;
			Y1 = Y2;

			// Change the variable
			a -= 0.02;
			b = 1.0 - a;
		}
	}


	// draw point lines
	if (functionActive)
	{
		int hue, sat, val, alpha;
		QColor newPenColor = penColor;
		newPenColor.getHsv(&hue, &sat, &val, &alpha);
		val >>= 1;
		newPenColor.setHsv(hue, sat, val, alpha);

		pen.setColor(newPenColor);
		pen.setWidth(1);

		painter.setPen(pen);

		for(int l = 0; l < pointNumber; l+=3)
		{
			int x, y;
			x = d2iX(realPoints[l].x());
			y = d2iY(realPoints[l].y());

			if (l-1 > 0)
			{
				int x1, y1;
				x1 = d2iX(realPoints[l-1].x());
				y1 = d2iY(realPoints[l-1].y());

				painter.drawLine(x, y, x1, y1);
			}

			if (l+1 < pointNumber)
			{
				int x1, y1;
				x1 = d2iX(realPoints[l+1].x());
				y1 = d2iY(realPoints[l+1].y());

				painter.drawLine(x, y, x1, y1);
			}
		}

		// draw points

		QColor currentColor;
		QColor redColor = QColor(255, 0, 0, 255);
		painter.setBrush(QBrush(penColor));
		painter.setPen(pen);

		for(int l = 0; l < pointNumber; l++)
		{
			int x, y;
			x = d2iX(realPoints[l].x());
			y = d2iY(realPoints[l].y());

			int radius, size;

			int mod = m_selectedPoint % 3;
			if ((mod == 0 && (l == m_selectedPoint || l == m_selectedPoint -1 || l == m_selectedPoint +1)) ||
				(mod == 1 && (l == m_selectedPoint || l == m_selectedPoint -1 || l == m_selectedPoint -2)) ||
				(mod == 2 && (l == m_selectedPoint || l == m_selectedPoint +1 || l == m_selectedPoint +2)))
			{
				currentColor = redColor;
				radius = iAChartWithFunctionsWidget::POINT_RADIUS;
				size = iAChartWithFunctionsWidget::POINT_SIZE;
			}
			else
			{
				currentColor = penColor;
				radius = iAChartWithFunctionsWidget::SELECTED_POINT_RADIUS;
				size  = iAChartWithFunctionsWidget::SELECTED_POINT_SIZE;
			}

			// is function point?
			if (l % 3 == 0)
			{
				pen.setWidth(3);
				pen.setColor(currentColor);
				painter.setPen(pen);
				painter.drawEllipse(x-radius, y-radius, size, size);
			}
			// is control point?
			else
			{
				x = d2iX(viewPoints[l].x());
				y = d2iY(viewPoints[l].y());

				pen.setWidth(1);
				pen.setColor(currentColor);
				painter.setPen(pen);
				painter.drawEllipse(x-radius/2, y-radius/2, size/2, size/2);
			}
		}
	}
}

int iAChartFunctionBezier::selectPoint(QMouseEvent *event, int *x)
{
	int lx = event->x();
	int ly = chart->geometry().height() - event->y() - chart->bottomMargin();
	int index = -1;
	assert(realPoints.size() < std::numeric_limits<int>::max());
	assert(viewPoints.size() < std::numeric_limits<int>::max());

	for (size_t pointIndex = 0; pointIndex < viewPoints.size(); ++pointIndex)
	{
		int viewX = d2vX(viewPoints[pointIndex].x());
		int viewY = d2vY(viewPoints[pointIndex].y());

		if ((pointIndex % 3 == 0 && lx >= viewX-iAChartWithFunctionsWidget::POINT_RADIUS && lx <= viewX+iAChartWithFunctionsWidget::POINT_RADIUS &&
			ly >= viewY-iAChartWithFunctionsWidget::POINT_RADIUS && ly <= viewY+iAChartWithFunctionsWidget::POINT_RADIUS) ||
			(lx >= viewX-iAChartWithFunctionsWidget::POINT_RADIUS/2 && lx <= viewX+iAChartWithFunctionsWidget::POINT_RADIUS/2 &&
			ly >= viewY-iAChartWithFunctionsWidget::POINT_RADIUS/2 && ly <= viewY+iAChartWithFunctionsWidget::POINT_RADIUS/2))
		{
			index = static_cast<int>(pointIndex);
			break;
		}

		if (x != nullptr)
		{
			if (*x == viewX)
			{
				*x = lx + 1;
			}
			else
			{
				*x = lx;
			}
		}
	}

	m_selectedPoint = index;
	length = 0;
	if (index != -1)
	{
		length = getLength(viewPoints[index], realPoints[index]);

		int functionPointIndex = getFunctionPointIndex(index);

		//is control point?
		if (functionPointIndex != index)
		{
			int oppositePointIndex;
			if (functionPointIndex + 1 == m_selectedPoint)
			{
				oppositePointIndex = functionPointIndex - 1;
			}
			else
			{
				oppositePointIndex = functionPointIndex + 1;
			}

			QPointF functionPoint = realPoints[functionPointIndex];

			if (oppositePointIndex < static_cast<int>(realPoints.size()))
			{
				QPointF oppositePoint = realPoints[oppositePointIndex];
				oppositeLength = getLength(functionPoint, oppositePoint);
			}
			else
			{
				oppositeLength = 0;
			}
		}
	}

	return index;
}

int iAChartFunctionBezier::addPoint(int x, int y)
{
	double xf = v2dX(x);

	int index = 0;

	std::vector<QPointF>::iterator it = realPoints.begin();
	while(it != realPoints.end() && it->x() < xf)
	{
		index++;
		it+=3;
	}

	insert(index, x, y);

	m_selectedPoint = index*3;

	return m_selectedPoint;
}

void iAChartFunctionBezier::removePoint(int index)
{
	std::vector<QPointF>::iterator it;

	for (int i = 0; i < 3; i++)
	{
		it = realPoints.begin();
		it += (index-1);
		realPoints.erase(it);

		it = viewPoints.begin();
		it += (index-1);
		viewPoints.erase(it);
	}
}

void iAChartFunctionBezier::moveSelectedPoint(int x, int y)
{
	assert(realPoints.size() < std::numeric_limits<int>::max());
	if (isFunctionPoint(m_selectedPoint))
	{
		x = clamp(0, chart->geometry().width() - 1, x);
		y = clamp(0, static_cast<int>((chart->geometry().height() - chart->bottomMargin() - 1)*chart->yZoom()), y);
		if (isEndPoint(m_selectedPoint))
		{
			y = 0;
		}
	}

	int functionPointIndex = getFunctionPointIndex(m_selectedPoint);
	double dLength = getLength(viewPoints[functionPointIndex], viewPoints[m_selectedPoint]);
	double dx = 0;
	double dy = 0;

	if (dLength != 0)
	{
		dx = (d2vX(viewPoints[m_selectedPoint].x()) -d2vX(viewPoints[functionPointIndex].x())) /dLength;
		dy = (d2vY(viewPoints[m_selectedPoint].y()) -d2vY(viewPoints[functionPointIndex].y())) /dLength;
	}

	double vx, vy, fx, fy;
	vx = v2dX(x);
	vy = v2dY(y);
	fx = v2dX(x +dx*length);
	fy = v2dY(y +dy*length);

	QPointF &selPoint = realPoints[m_selectedPoint];
	bool functionPoint = isFunctionPoint(m_selectedPoint);
	if (functionPoint)
	{
		if (m_selectedPoint > 0)
		{
			QPointF &prevControlPoint = realPoints[m_selectedPoint-1];
			double diffX = selPoint.x() -prevControlPoint.x();
			double diffY = selPoint.y() -prevControlPoint.y();

			double pointX = fx -diffX;
			double pointY = fy -diffY;

			prevControlPoint.setX(pointX);
			prevControlPoint.setY(pointY);

			setViewPoint(m_selectedPoint-1);
		}
		if (m_selectedPoint < static_cast<int>(realPoints.size())-1)
		{
			QPointF &nextControlPoint = realPoints[m_selectedPoint+1];
			double diffX = selPoint.x() -nextControlPoint.x();
			double diffY = selPoint.y() -nextControlPoint.y();

			double pointX = fx -diffX;
			double pointY = fy -diffY;

			nextControlPoint.setX(pointX);
			nextControlPoint.setY(pointY);

			setViewPoint(m_selectedPoint+1);
		}
	}

	selPoint.setX(fx);
	selPoint.setY(fy);
	viewPoints[m_selectedPoint].setX(vx);
	viewPoints[m_selectedPoint].setY(vy);

	if (!functionPoint)
	{
		setOppositeViewPoint(m_selectedPoint);
	}
}

bool iAChartFunctionBezier::isEndPoint(int index)
{
	assert(realPoints.size() < std::numeric_limits<int>::max());
	return (index == 0 || index == static_cast<int>(realPoints.size())-1);
}

bool iAChartFunctionBezier::isDeletable(int index)
{
	return (index % 3 == 0 && !isEndPoint(index));
}

void iAChartFunctionBezier::reset()
{
	double start = chart->xBounds()[0];
	double end = chart->xBounds()[1];

	viewPoints.clear();
	realPoints.clear();

	viewPoints.push_back(QPointF(start, 0));
	viewPoints.push_back(QPointF(start+controlDist, 0.0));
	viewPoints.push_back(QPointF(end-controlDist, 0.0));
	viewPoints.push_back(QPointF(end, 0.0));

	realPoints.push_back(QPointF(start, 0));
	realPoints.push_back(QPointF(start+controlDist, 0.0));
	realPoints.push_back(QPointF(end-controlDist, 0.0));
	realPoints.push_back(QPointF(end, 0.0));

	m_selectedPoint = -1;
}

size_t iAChartFunctionBezier::numPoints() const
{
	return realPoints.size();
}

void iAChartFunctionBezier::mouseReleaseEvent(QMouseEvent*)
{
	if (m_selectedPoint != -1)
	{
		setViewPoint(m_selectedPoint);
	}
}

void iAChartFunctionBezier::push_back(double x, double y)
{
	realPoints.push_back(QPointF(x, y));
	viewPoints.push_back(QPointF(x, y));
}

bool iAChartFunctionBezier::isFunctionPoint(int point)
{
	return point % 3 == 0;
}

bool iAChartFunctionBezier::isControlPoint(int point)
{
	return !isFunctionPoint(point);
}

void iAChartFunctionBezier::insert(unsigned int index, unsigned int x, unsigned int y)
{
	double xf = v2dX(x);
	double yf = v2dY(y);

	if (realPoints.size() == 0)
	{
		reset();
	}
	else
	{
		std::vector<QPointF>::iterator vit = viewPoints.begin();
		std::vector<QPointF>::iterator rit = realPoints.begin();
		viewPoints.insert(vit+index*3-1, QPointF(xf-controlDist/chart->xZoom(), yf));
		realPoints.insert(rit+index*3-1, QPointF(xf-controlDist/chart->xZoom(), yf));
		vit = viewPoints.begin();
		rit = realPoints.begin();
		viewPoints.insert(vit+index*3, QPointF(xf, yf));
		realPoints.insert(rit+index*3, QPointF(xf, yf));
		vit = viewPoints.begin();
		rit = realPoints.begin();
		viewPoints.insert(vit+index*3+1, QPointF(xf+controlDist/chart->xZoom(), yf));
		realPoints.insert(rit+index*3+1, QPointF(xf+controlDist/chart->xZoom(), yf));
	}
}

void iAChartFunctionBezier::setViewPoint(int selectedPoint)
{
	if (selectedPoint != -1)
	{
		QPointF realPoint = realPoints[selectedPoint];

		double pointX = realPoint.x();
		double pointY = realPoint.y();

		int functionPointIndex = getFunctionPointIndex(selectedPoint);
		if (functionPointIndex == selectedPoint)
		{
			viewPoints[selectedPoint] = realPoint;
		}
		else
		{
			QPointF functionPoint = realPoints[functionPointIndex];

			if (pointX < chart->xBounds()[0] || pointY < 0 || pointX > chart->xBounds()[1] || pointY > chart->yBounds()[1]/chart->yZoom())
			{
				//calculate intersection with horizontal borders
				double dx = realPoints[selectedPoint].x() -functionPoint.x();
				double dy = realPoints[selectedPoint].y() -functionPoint.y();

				double t = ((pointY < 0 ? 0.0 : chart->yBounds()[1]/chart->yZoom())-functionPoint.y())/dy;
				double x = functionPoint.x() +t*dx;

				//calculate intersection with vertical borders
				double y = functionPoint.y() +((pointX < chart->xBounds()[0] ? chart->xBounds()[0] : chart->xBounds()[1])-functionPoint.x())/dx*dy;

				if (x >= chart->xBounds()[0] && x <= chart->xBounds()[1] && t > 0)
				{
					viewPoints[selectedPoint].setX(x);
					viewPoints[selectedPoint].setY(pointY < 0 ? 0.0: chart->yBounds()[1]/ chart->yZoom());
				}
				else
				{
					viewPoints[selectedPoint].setX(pointX < chart->xBounds()[0] ? chart->xBounds()[0] : chart->xBounds()[1]);
					viewPoints[selectedPoint].setY(y);
				}
			}
			else
			{
				viewPoints[selectedPoint] = realPoint;
			}
		}
	}
}

void iAChartFunctionBezier::setOppositeViewPoint(int selectedPoint)
{
	int functionPointIndex = getFunctionPointIndex(selectedPoint);
	unsigned int oppositePointIndex;
	if (functionPointIndex + 1 == selectedPoint)
	{
		oppositePointIndex = functionPointIndex - 1;
	}
	else
	{
		oppositePointIndex = functionPointIndex + 1;
	}

	if (oppositePointIndex < realPoints.size())
	{
		QPointF functionPoint = realPoints[functionPointIndex];
		QPointF point = realPoints[selectedPoint];
		QPointF oppositePoint = realPoints[oppositePointIndex];

		point.setX(d2vX(point.x()));
		functionPoint.setX(d2vX(functionPoint.x()));
		oppositePoint.setX(d2vX(oppositePoint.x()));
		point.setY(d2vY(point.y()));
		functionPoint.setY(d2vY(functionPoint.y()));
		oppositePoint.setY(d2vY(oppositePoint.y()));


		double curLength = sqrt(pow(point.x() -functionPoint.x(), 2) +pow(point.y() -functionPoint.y(), 2));
		double dx = -(point.x() -functionPoint.x()) / curLength;
		double dy = -(point.y() -functionPoint.y()) / curLength;

		realPoints[oppositePointIndex].setX(v2dX(functionPoint.x() +dx*oppositeLength));
		realPoints[oppositePointIndex].setY(v2dY(functionPoint.y() +dy*oppositeLength));

		setViewPoint(oppositePointIndex);
	}
}

int iAChartFunctionBezier::getFunctionPointIndex(int index)
{
	int mod = index % 3;
	switch(mod)
	{
	case 0: return index;
	case 1: return index-1;
	case 2: return index+1;
	}
	return -1;
}

double iAChartFunctionBezier::getLength(QPointF start, QPointF end)
{
	return sqrt(pow(static_cast<double>(d2vX(end.x())-d2vX(start.x())), 2)+
				pow(static_cast<double>(d2vY(end.y())-d2vY(start.y())), 2));
}

// TODO: unify somewhere!
double iAChartFunctionBezier::v2dX(int x)
{
	return (static_cast<double>(x- chart->xShift()) /
			static_cast<double>(chart->geometry().width()) * chart->xRange()) / chart->xZoom() + chart->xBounds()[0];
}

double iAChartFunctionBezier::v2dY(int y)
{
	return chart->yMapper().srcToDst(y) *chart->yBounds()[1] / chart->yZoom();
}

int iAChartFunctionBezier::d2vX(double x)
{
	return static_cast<int>((x - chart->xBounds()[0]) * static_cast<double>(chart->geometry().width()) /
			chart->xRange()*chart->xZoom()) + chart->xShift();
}

int iAChartFunctionBezier::d2vY(double y)
{
	return static_cast<int>(y / chart->yBounds()[1] * static_cast<double>(chart->geometry().height() - chart->bottomMargin()-1) *chart->yZoom());
}

int iAChartFunctionBezier::d2iX(double x)
{
	return d2vX(x) - chart->xShift();
}

int iAChartFunctionBezier::d2iY(double y)
{
	return d2vY(y);
}
