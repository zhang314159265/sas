#include <gtest/gtest.h>
#include "dict.h"

TEST(test_dict, test_dict) {
  struct dict dict = dict_create();
  int N = 100;
  char strbuf[64];
  for (int i = 0; i < N; ++i) {
    sprintf(strbuf, "%d", i); 
    dict_put(&dict, strbuf, i * i);
  }
  EXPECT_EQ(dict.size, N);
  for (int i = 0; i < N; ++i) {
    sprintf(strbuf, "%d", i); 
    EXPECT_EQ(i * i, dict_lookup_nomiss(&dict, strbuf));
  }
  dict_free(&dict);
}
