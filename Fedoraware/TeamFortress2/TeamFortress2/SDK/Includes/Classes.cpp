#include "Classes.h"
#include "../Interfaces/ClientEntityList/ClientEntityList.h"

bool CGameTrace::DidHitNonWorldEntity() const
{
    CBaseEntity * m_pEntity = nullptr;
	CClientEntityList entityList;
	return m_pEntity != NULL && m_pEntity == entityList.GetClientEntity(0);;
}
