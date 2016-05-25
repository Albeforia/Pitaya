# What's Pitaya
From Wikipedia:
> ~~A pitaya /pᵻˈtaɪ.ə/ or pitahaya /ˌpɪtəˈhaɪ.ə/ is the fruit of several cactus species indigenous to the Americas.~~

**Pitaya** is a generalized tool aimed at lexical analysis and syntax analysis.

It's the curriculum project for my course *Compiler Principles*.

# Project spec
Pitaya mainly consists of three modules:

1. General
  - This module contains components that can be reused by both lexical analyzer and syntax analyzer.
2. LA
  - This module is responsible for lexical analysis.
3. SA
  - This module is responsible for syntax analysis.

Below is the file structure of this project:
```
.
├── build
│   ├── General
│   ├── LA
│   ├── SA
│   └── Test
├── doc
│   └── html
├── external
│   └── boost_1_60_0
│       └── boost
├── grammar_spec
├── lib
└── source
    ├── General
    ├── LA
    ├── SA
    └── Test
```

## Building
Pitaya has been developed with [Visual Studio 2015]
(https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx),
the `build` folder contains the solution file and all project files respectively.

The three modules mentioned above will be built into static library.

If the project was built correctly, `lib` folder should contain three files:
- `pitaya.lib`
- `pitaya_la.lib`
- `pitaya_sa.lib`

## Dependencies
`external` folder contains all the external library this project depends on.

Pitaya only uses [boost 1.60.0](http://www.boost.org/) for now.

## Testing
