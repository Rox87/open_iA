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
#include "iAFoamCharacterizationTable.h"

#include "iAFoamCharacterizationItemBinarization.h"
#include "iAFoamCharacterizationItemDistanceTransform.h"
#include "iAFoamCharacterizationItemFilter.h"
#include "iAFoamCharacterizationItemWatershed.h"

#include <vtkImageData.h>

#include <QDropEvent>
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>

iAFoamCharacterizationTable::iAFoamCharacterizationTableDelegate::iAFoamCharacterizationTableDelegate(iAFoamCharacterizationTable* _pTable, QObject* _pParent)
	: QItemDelegate(_pParent)
	, m_pTable(_pTable)
{
	m_iMargin = 100 * m_pTable->logicalDpiX() / 254;
}

void iAFoamCharacterizationTable::iAFoamCharacterizationTableDelegate::paint(QPainter* _pPainter, const QStyleOptionViewItem& _sovItem, const QModelIndex& _miItem) const
{
	const int iRow(_miItem.row());

	QModelIndexList mlIndex(m_pTable->selectedIndexes());

	if ((mlIndex.size()) && (m_pTable->hasFocus()))
	{
		const int iRowSelected(mlIndex.at(0).row());

		if (iRow == iRowSelected)
		{
			drawItemRect(_pPainter, _sovItem.rect, m_pTable->palette().color(QPalette::Highlight));
			_pPainter->setPen(m_pTable->palette().color(QPalette::BrightText));
		}
		else
		{
			drawItemRect(_pPainter, _sovItem.rect, m_pTable->palette().color(QPalette::Window));
			_pPainter->setPen(m_pTable->palette().color(QPalette::WindowText));
		}
	}
	else
	{
		drawItemRect(_pPainter, _sovItem.rect, m_pTable->palette().color(QPalette::Window));
		_pPainter->setPen(m_pTable->palette().color(QPalette::WindowText));
	}

	iAFoamCharacterizationItem* pItem((iAFoamCharacterizationItem*)m_pTable->item(iRow, 0));

	const QRect rText(_sovItem.rect.adjusted(m_iMargin, 0, -m_iMargin, 0));

	QIcon iItemIcon(pItem->icon());

	const QSize sItemIcon(_sovItem.decorationSize);

	_pPainter->drawPixmap((rText.left() - sItemIcon.width()) / 2
		, rText.top() + (rText.height() - sItemIcon.height()) / 2
		, iItemIcon.pixmap(sItemIcon.width(), sItemIcon.height())
	);

	_pPainter->setFont(pItem->font());

	_pPainter->drawText
	(rText.adjusted(m_iMargin / 4, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, _miItem.data().toString());

	if (pItem->executing())
	{
		const int iProgress(pItem->progress());

		if (iProgress > 0)
		{
			const QRect rProgress(_sovItem.rect.width() - 3 * m_iMargin
				, _sovItem.rect.top() + _sovItem.rect.height() / 4
				, 2 * m_iMargin, _sovItem.rect.height() / 2
			);

			_pPainter->drawRect(rProgress);

			QRect rBar(rProgress.adjusted(1, 1, 0, 0));
			rBar.setWidth(rBar.width() * iProgress / 100);

			QLinearGradient lg(rBar.topLeft(), rBar.topRight());
			lg.setColorAt(0.0, Qt::black);
			lg.setColorAt(1.0, pItem->itemIconColor());

			_pPainter->fillRect(rBar, QBrush(lg));
		}
	}
	else
	{
		const QString sTime(pItem->executeTimeString());

		_pPainter->setPen((pItem->modified()) ? colorModified(_pPainter->pen().color()) : _pPainter->pen().color());
		_pPainter->drawText(rText, Qt::AlignRight | Qt::AlignVCenter, sTime);
	}
}

QColor iAFoamCharacterizationTable::iAFoamCharacterizationTableDelegate::colorModified(const QColor& _cColor) const
{
	return ((_cColor.lightness() > 200) ? _cColor.darker(125) : Qt::gray);
}

void iAFoamCharacterizationTable::iAFoamCharacterizationTableDelegate::drawItemRect(QPainter* _pPainter, const QRect& _rItem, const QColor& _cColor) const
{
	_pPainter->setBrush(colorModified(_cColor));
	_pPainter->setPen(_cColor);
	_pPainter->drawRect(_rItem.adjusted(0, 0, m_iMargin - _rItem.width(), -1));
	_pPainter->fillRect(_rItem.adjusted(m_iMargin, 0, 0, 0), _cColor);
}



iAFoamCharacterizationTable::iAFoamCharacterizationTable(vtkImageData* _pImageData, QWidget* _pParent)
																			  : QTableWidget(_pParent), m_pImageData (_pImageData)
{
	setAutoFillBackground(false);
	setCursor(Qt::PointingHandCursor);

	setDragDropMode(QAbstractItemView::InternalMove);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);

	horizontalHeader()->setSectionsClickable(false);

	verticalHeader()->setSectionsClickable(false);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	setColumnCount(1);

	const QStringList slLabels("Foam characterization pipeline");
	setHorizontalHeaderLabels(slLabels);

	setItemDelegate(new iAFoamCharacterizationTableDelegate(this, this));
}

void iAFoamCharacterizationTable::addBinarization()
{
	const int n(rowCount());

	setRowCount(n + 1);

	++m_iCountBinarization;

	iAFoamCharacterizationItemBinarization* pItem(new iAFoamCharacterizationItemBinarization(this, m_pImageData));
	pItem->setName(pItem->text() + QString(" %1").arg(m_iCountBinarization));
	setItem(n, 0, pItem);
}

void iAFoamCharacterizationTable::addDistanceTransform()
{
	const int n(rowCount());

	setRowCount(n + 1);

	++m_iCountDistanceTransform;

	iAFoamCharacterizationItemDistanceTransform* pItem(new iAFoamCharacterizationItemDistanceTransform(this, m_pImageData));
	pItem->setName(pItem->text() + QString(" %1").arg(m_iCountDistanceTransform));
	setItem(n, 0, pItem);
}

void iAFoamCharacterizationTable::addFilter()
{
	const int n(rowCount());

	setRowCount(n + 1);

	++m_iCountFilter;

	iAFoamCharacterizationItemFilter* pItem(new iAFoamCharacterizationItemFilter(this, m_pImageData));
	pItem->setName(pItem->text() + QString(" %1").arg(m_iCountFilter));
	setItem(n, 0, pItem);
}

void iAFoamCharacterizationTable::addWatershed()
{
	const int n(rowCount());

	setRowCount(n + 1);

	++m_iCountWatershed;

	iAFoamCharacterizationItemWatershed* pItem(new iAFoamCharacterizationItemWatershed(this, m_pImageData));
	pItem->setName(pItem->text() + QString(" %1").arg(m_iCountWatershed));
	setItem(n, 0, pItem);
}

void iAFoamCharacterizationTable::clear()
{
	while (rowCount())
	{
		removeRow(0);
	}

	m_iCountBinarization = 0;
	m_iCountDistanceTransform = 0;
	m_iCountFilter = 0;
	m_iCountWatershed = 0;
}

void iAFoamCharacterizationTable::dropEvent(QDropEvent* e)
{
	if ((e->source() == this) && (m_iRowDrag > -1))
	{
		m_iRowDrop = indexAt(e->pos()).row();

		if ((m_iRowDrop > -1) && (m_iRowDrag != m_iRowDrop))
		{
			iAFoamCharacterizationItem* pItemDrag((iAFoamCharacterizationItem*) item(m_iRowDrag, 0));

			if (pItemDrag->itemType() == iAFoamCharacterizationItem::itBinarization)
			{
				pItemDrag =
					new iAFoamCharacterizationItemBinarization((iAFoamCharacterizationItemBinarization*) takeItem(m_iRowDrag, 0));
			}
			else if (pItemDrag->itemType() == iAFoamCharacterizationItem::itDistanceTransform)
			{
				pItemDrag = new iAFoamCharacterizationItemDistanceTransform
				                                          ((iAFoamCharacterizationItemDistanceTransform*)takeItem(m_iRowDrag, 0));
			}
			else if (pItemDrag->itemType() == iAFoamCharacterizationItem::itFilter)
			{
				pItemDrag = new iAFoamCharacterizationItemFilter((iAFoamCharacterizationItemFilter*)takeItem(m_iRowDrag, 0));
			}
			else
			{
				pItemDrag =
					      new iAFoamCharacterizationItemWatershed((iAFoamCharacterizationItemWatershed*) takeItem(m_iRowDrag, 0));
			}

			if (m_iRowDrag < m_iRowDrop)
			{
				int ii(m_iRowDrag + 1);

				for (int i(m_iRowDrag); i < m_iRowDrop; ++i, ++ii)
				{
					setItem(i, 0, (iAFoamCharacterizationItem*) takeItem(ii, 0));
				}
			}
			else
			{
				int ii(m_iRowDrag - 1);

				for (int i(m_iRowDrag); i > m_iRowDrop; --i, --ii)
				{
					setItem(i, 0, (iAFoamCharacterizationItem*) takeItem(ii, 0));
				}
			}

			setItem(m_iRowDrop, 0, pItemDrag);

			if (pItemDrag->itemType() == iAFoamCharacterizationItem::itBinarization)
			{
				const int n(rowCount());

				for (int i(0); i < n; ++i)
				{
					iAFoamCharacterizationItem* pItem((iAFoamCharacterizationItem*)item(i, 0));

					if (pItem->itemType() == iAFoamCharacterizationItem::itDistanceTransform)
					{
						iAFoamCharacterizationItemDistanceTransform* pDistanceTransform
						                                                   ((iAFoamCharacterizationItemDistanceTransform*) pItem);

						if (pDistanceTransform->itemMask() == m_iRowDrag)
						{
							pDistanceTransform->setItemMask(m_iRowDrop);
						}
					}

					if (pItem->itemType() == iAFoamCharacterizationItem::itWatershed)
					{
						iAFoamCharacterizationItemWatershed* pWatershed ((iAFoamCharacterizationItemWatershed*)pItem);

						if (pWatershed->itemMask() == m_iRowDrag)
						{
							pWatershed->setItemMask(m_iRowDrop);
						}
					}
				}
			}

		}
	}
}

void iAFoamCharacterizationTable::execute()
{
	reset();

	setFocus();

	const int n(rowCount());

	for (int i (0) ; i < n ; ++i)
	{
		selectRow(i);
		viewport()->repaint();

		iAFoamCharacterizationItem* pItem((iAFoamCharacterizationItem*)item(i, 0));

		if (pItem->itemEnabled())
		{
			pItem->execute();
		}
	}

	viewport()->repaint();
}

void iAFoamCharacterizationTable::keyPressEvent(QKeyEvent* e)
{
	QModelIndexList mlIndex(selectedIndexes());

	if (mlIndex.size())
	{
		const int iRowSelected(mlIndex.at(0).row());

		if (e->key() == Qt::Key_Delete)
		{
			if ( QMessageBox::information ( this, "Information", "Delete " + item(iRowSelected, 0)->text() + "?"
				                          , QMessageBox::Yes, QMessageBox::No
			                              ) == QMessageBox::Yes
			   )
			{
				removeRow(iRowSelected);
			}

			e->accept();
		}
		else if ((e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return))
		{
			((iAFoamCharacterizationItem*)item(mlIndex.at(0).row(), 0))->dialog();

			e->accept();
		}
	}

	QTableWidget::keyPressEvent(e);
}

void iAFoamCharacterizationTable::mouseDoubleClickEvent(QMouseEvent* e)
{
	QTableWidget::mouseDoubleClickEvent(e);

	const int iMargin(100 * logicalDpiX() / 254);

	if (e->x() > iMargin)
	{
		QModelIndexList mlIndex(selectedIndexes());

		if (mlIndex.size())
		{
			iAFoamCharacterizationItem* pItem((iAFoamCharacterizationItem*) item(mlIndex.at(0).row(), 0));

			pItem->dialog();
		}
	}
}

void iAFoamCharacterizationTable::mousePressEvent(QMouseEvent* e)
{
	const QPoint ptMouse (e->pos());

	m_iRowDrag = indexAt(ptMouse).row();

	if (m_iRowDrag > -1)
	{
		iAFoamCharacterizationItem* pItem((iAFoamCharacterizationItem*) item(m_iRowDrag, 0));

		const int iMargin(100 * logicalDpiX() / 254);

		if (ptMouse.x() < iMargin)
		{
			pItem->setItemEnabled(!pItem->itemEnabled());
		}
	}

	QTableWidget::mousePressEvent(e);
}

void iAFoamCharacterizationTable::mouseReleaseEvent(QMouseEvent* e)
{
	QTableWidget::mouseReleaseEvent(e);

	viewport()->repaint();
}

void iAFoamCharacterizationTable::open(const QString& _sFilename)
{
	QScopedPointer<QFile> pFileOpen(new QFile(_sFilename));

	if (pFileOpen->open(QIODevice::ReadOnly))
	{
		clear();

		int n (0);
		pFileOpen->read((char*)&n, sizeof(n));

		for (int i(0); i < n; ++i)
		{
			iAFoamCharacterizationItem::EItemType eItemType;
			pFileOpen->read((char*) &eItemType, sizeof(eItemType));

			switch (eItemType)
			{
				case iAFoamCharacterizationItem::itBinarization:
				addBinarization();
				break;

				case iAFoamCharacterizationItem::itFilter:
				addFilter();
				break;

				case iAFoamCharacterizationItem::itDistanceTransform:
				addDistanceTransform();
				break;

				default:
				addWatershed();
				break;
			}

			((iAFoamCharacterizationItem*)item(i, 0))->open(pFileOpen.data());
		}

		pFileOpen->close();
	}
}

void iAFoamCharacterizationTable::reset()
{
	const int n(rowCount());

	for (int i(0); i < n; ++i)
	{
		((iAFoamCharacterizationItem*)item(i, 0))->reset();
	}

	viewport()->repaint();
}

void iAFoamCharacterizationTable::resizeEvent(QResizeEvent* e)
{
	setColumnWidth(0, viewport()->width());

	QTableWidget::resizeEvent(e);
}

void iAFoamCharacterizationTable::save(const QString& _sFilename)
{
	QScopedPointer<QFile> pFileSave(new QFile(_sFilename));

	if (pFileSave->open(QIODevice::WriteOnly))
	{
		const int n(rowCount());

		pFileSave->write((char*) &n, sizeof(n));

		for (int i(0); i < n; ++i)
		{
			((iAFoamCharacterizationItem*)item(i, 0))->save(pFileSave.data());
		}

		pFileSave->close();
	}
}
