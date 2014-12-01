{

    'includes': [
        'gyp-config/cpp11.gypi',
    ],

    'targets': [
        {
            'target_name': 'winksignal',
            'type': '<(component)',

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
            },

        },
    ],

    'conditions': [
        ['OS=="linux"', {
            'targets': [
                {
                    'target_name': 'winksignal_test',
                    'type': 'executable',

                    'include_dirs':[
                        'test/catch/include',
                        'source',
                    ],

                    'sources': [
                        '<!@(ls test/*.cc)',
                    ],

                    'dependencies': [
                        'winksignal',
                    ],

                },
            ],
        }],
    ],   # conditions

}
