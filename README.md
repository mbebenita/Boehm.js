## Compile

```
emconfigure ./configure --without-threads --disable-threads __EMSCRIPTEN__=1
emmake make
emcc -Iinclude/ .libs/libgc.dylib demo.cpp -o demo.js
```

N.B.: You need to use the correct extension for libgc according to which platform you're using to build.
