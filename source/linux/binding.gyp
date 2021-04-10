{
  "targets": [
    {
      "target_name": "sharememory",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "sharememory.cc" ]
    }
  ]
}
