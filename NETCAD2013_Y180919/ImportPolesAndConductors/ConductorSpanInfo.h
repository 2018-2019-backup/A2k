#pragma once
#include "CircuitInfo.h"

class CConductorSpanInfo
{
	public:
		CConductorSpanInfo(void);
		~CConductorSpanInfo(void);

	ads_point m_ptInsert;

	std::vector <CCircuitInfo> m_CircuitInfoVetor;
};

