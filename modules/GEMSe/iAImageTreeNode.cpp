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
#include "iAImageTreeNode.h"

iAImageTreeNode::iAImageTreeNode() :
	m_attitude(NoPreference)
{
}

void iAImageTreeNode::SetParent(QSharedPointer<iAImageTreeNode > parent)
{
	m_parent = parent;
}

QSharedPointer<iAImageTreeNode > iAImageTreeNode::GetParent() const
{
	return m_parent;
}

iAImageTreeNode::Attitude iAImageTreeNode::GetAttitude() const
{
	return m_attitude;
}

iAImageTreeNode::Attitude iAImageTreeNode::ParentAttitude() const
{
	return GetParent() ?
		(GetParent()->GetAttitude() != NoPreference ?
			GetParent()->GetAttitude() :
			GetParent()->ParentAttitude()
			) :
		NoPreference;
}

void iAImageTreeNode::SetAttitude(Attitude att)
{
	m_attitude = att;
}


void iAImageTreeNode::ClearFilterData()
{
}


void FindNode(iAImageTreeNode const * searched, QList<QSharedPointer<iAImageTreeNode> > & path, QSharedPointer<iAImageTreeNode> curCluster, bool & found)
{
	path.push_back(curCluster);
	if (curCluster.data() != searched)
	{
		for (int i = 0; i<curCluster->GetChildCount() && !found; ++i)
		{
			FindNode(searched, path, curCluster->GetChild(i), found);
		}
		if (!found)
		{
			path.removeLast();
		}
	}
	else
	{
		found = true;
	}
}

QSharedPointer<iAImageTreeNode> GetSibling(QSharedPointer<iAImageTreeNode> node)
{
	QSharedPointer<iAImageTreeNode> parent(node->GetParent());
	for (int i = 0; i<parent->GetChildCount(); ++i)
	{
		if (parent->GetChild(i) != node)
		{
			return parent->GetChild(i);
		}
	}
	return QSharedPointer<iAImageTreeNode>();
}