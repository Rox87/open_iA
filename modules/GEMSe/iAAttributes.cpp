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
#include "iAAttributes.h"

#include <iAAttributeDescriptor.h>

#include <QTextStream>

QSharedPointer<iAAttributes> iAAttributes::create(QTextStream & in)
{
	QSharedPointer<iAAttributes> result(new iAAttributes);
	while (!in.atEnd())
	{
		QString line = in.readLine();
		QSharedPointer<iAAttributeDescriptor> descriptor =
			iAAttributeDescriptor::create(line);
		if (descriptor)
		{
			result->m_attributes.push_back(descriptor);
		}
		else
		{
			return QSharedPointer<iAAttributes>(new iAAttributes);
		}
	}
	return result;
}

int iAAttributes::size() const
{
	return m_attributes.size();
}


QSharedPointer<iAAttributeDescriptor> iAAttributes::at(int idx)
{
	return m_attributes[idx];
}

QSharedPointer<iAAttributeDescriptor const> iAAttributes::at(int idx) const
{
	return m_attributes[idx];
}


void iAAttributes::add(QSharedPointer<iAAttributeDescriptor> range)
{
	m_attributes.push_back(range);
}

void iAAttributes::store(QTextStream & out)
{
	for (int i = 0; i < m_attributes.size(); ++i)
	{
		out << m_attributes[i]->toString();
	}
}

int iAAttributes::find(QString const & name)
{
	for (int i = 0; i < m_attributes.size(); ++i)
	{
		if (m_attributes[i]->name() == name)
		{
			return i;
		}
	}
	return -1;
}


int iAAttributes::count(iAAttributeDescriptor::iAAttributeType type) const
{
	int count = 0;
	for (int i = 0; i < m_attributes.size(); ++i)
	{
		if (type == iAAttributeDescriptor::None	|| m_attributes[i]->attribType() == type)
		{
			count++;
		}
	}
	return count;
}
