#ifndef _TEST_PAGEPERM_H
#define _TEST_PAGEPERM_H

#include "jitmenot.h"

int register_test_pageperm(struct test_chain *);

#define TEST_ID_PAGEPERM 4
#define TEST_NAME_PAGEPERM "pageperm"
#define TEST_DESC_PAGEPERM "Checks for pages with RWX permissions."

#endif
