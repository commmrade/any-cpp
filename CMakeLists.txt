cmake_minimum_required(VERSION 3.12)
project(proj VERSION 0.1.0)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Поиск установленного Google Test ---
# CMake будет искать конфигурационные файлы Google Test,
# которые обычно устанавливаются вместе с библиотекой.
find_package(GTest REQUIRED)
# Если CMake не может найти GTest, вам может потребоваться указать CMAKE_PREFIX_PATH
# при вызове CMake, например: cmake -DCMAKE_PREFIX_PATH=/путь/к/установке/gtest ..
# или добавить его в переменные окружения.
# --- Конец поиска Google Test ---

# Основной исполняемый файл
add_executable(proj main.cpp)

# --- Настройка для тестов ---
enable_testing() # Включаем тестирование в CMake

# Пример исполняемого файла для тестов
# Предположим, у вас есть файл test/main_test.cpp с вашими тестами
# add_executable(run_tests test/main_test.cpp) # Замените test/main_test.cpp на ваш файл с тестами

# Связываем тесты с найденным Google Test
# GTest::gtest_main включает как библиотеку gtest, так и функцию main().
# Если вы хотите предоставить свою собственную функцию main(), используйте GTest::gtest
# и GTest::gmock (если нужен) и напишите свой main.
# target_link_libraries(run_tests PRIVATE GTest::gtest_main)
# Если вы также используете Google Mock:
# target_link_libraries(run_tests PRIVATE GTest::gtest_main GTest::gmock_main)

# Добавляем тест в CTest
include(GoogleTest) # Необходимо для gtest_discover_tests
# gtest_discover_tests(run_tests)
# Альтернативно, если gtest_discover_tests недоступен или нежелателен:
# add_test(NAME MyProjectTests COMMAND run_tests)

# Вы также можете захотеть связать вашу основную библиотеку (если она есть) с тестами,
# чтобы тестировать ее функциональность.
# Например, если бы 'proj' был библиотекой:
# target_link_libraries(run_tests PRIVATE proj)