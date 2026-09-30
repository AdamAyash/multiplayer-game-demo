#pragma once

#include "Windows.h"

#define SECURE_ZERO_MEMORY() SecureZeroMemory(this, sizeof(*this));
