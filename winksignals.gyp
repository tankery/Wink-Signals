{

    'targets': [
        {
            'target_name': 'winksignal_test',
            'type': 'executable',

            'include_dirs':[
                'test/catch/include',
                '.',
            ],

            'sources': [
                '<!@(ls test/*.cc)',
            ],

        },
    ],

    'target_defaults': {

        'cflags': [
            '-std=c++11',
            '-O3',
        ],

        'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11','-stdlib=libc++'],
            'OTHER_LDFLAGS': ['-stdlib=libc++'],
        },
    },

}
