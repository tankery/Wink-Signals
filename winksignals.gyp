{

    'includes': [
        'gyp-config/cpp11.gypi',
    ],

    'targets': [
        {
            'target_name': 'winksignals',
            'type': '<(component)',

            'defines': [
                'WINK_MULTI_THREAD',
            ],

            'include_dirs':[
                'include',
                'source',
            ],

            'sources': [
                '<!@(ls source/*.cc)',
            ],

            'direct_dependent_settings': {
                'include_dirs':[
                    'include',
                ],
                'defines': [
                    'WINK_MULTI_THREAD',
                ],
            },

        },
    ],

    'conditions': [
        ['OS=="linux"', {
            'targets': [
                {
                    'target_name': 'winksignals_test',
                    'type': 'executable',

                    'include_dirs':[
                        'test/catch/include',
                        'source',
                    ],

                    'sources': [
                        '<!@(ls test/*.cc)',
                    ],

                    'dependencies': [
                        'winksignals',
                    ],

                },
            ],
        }],
    ],   # conditions

}
