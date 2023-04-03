{
  "targets": [
    {
      "target_name": "tsfn_test",
      "sources": [
        "tsfn_test.cc",
      ],
      "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
    },
    {
      "target_name": "tsfn_test_c",
      "sources": [
        "tsfn_test_c.c",
      ],
    }
  ]
}
