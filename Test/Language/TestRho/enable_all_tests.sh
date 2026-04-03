#\!/bin/bash

# Enable all disabled tests in TestRho directory

# Handle TEST(DISABLED_TestName, ...) pattern
find . -name '*.cpp' -not -path './old_backups/*' -exec sed -i 's/TEST(DISABLED_/TEST(/g' {} \;
find . -name '*.cpp' -not -path './old_backups/*' -exec sed -i 's/TEST_F(DISABLED_/TEST_F(/g' {} \;

# Handle TEST(TestName, DISABLED_TestCase) pattern  
find . -name '*.cpp' -not -path './old_backups/*' -exec sed -i 's/, DISABLED_/, /g' {} \;

echo 'All DISABLED_ prefixes have been removed from test names.'

