# CZI Pyramidizer

A tool for generating and verifying multi-resolution pyramids inside CZI image containers.

## Features

- Detect whether a pyramid in a given CZI document is required
- Detect whether an pyramid exists already
- Generate a multi-resolution pyramid and add it to the CZI

## Quick Start

```
czi-pyramidizer --source input.czi --destination output.czi
```

Check only:
```
czi-pyramidizer --check-only input.czi
```

## Status

Early-stage documentation. Contributions welcome.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
