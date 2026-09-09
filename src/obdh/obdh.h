#pragma once

#include "csp/csp.h"
#include "csp/drivers/usart.h"
#include "obdh/obdh_config.h"

/// @brief initialize the obdh task
/// @param address the csp address
/// @return 0 on success, otherwise an error
int obdh_init(int address);
