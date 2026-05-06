
incSrcTest = include_directories(
                    '../src'
                  , '..')   # for ...config.h

fract_test_src = files('fract_test.cpp')
fract_test = executable('fract_test'
    , fract_test_src
    , dependencies: deps
    , include_directories : incSrcTest
    , link_with: [life_lib]
    )
test('fract_test', fract_test)

