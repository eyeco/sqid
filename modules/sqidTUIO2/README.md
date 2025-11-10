# sqıd Plugin for TUIO2

This repository contains the source code for a [TUIO 2](https://www.tuio.org/?tuio20) plugin for the [sqıd visual programming environment](https://github.com/eyeco/sqid). It builds on the [TUIO 2.0 C++ Library](https://github.com/mkalten/TUIO20_CPP) in order to receive TUIO pointers and (not yet implemented) objects and blobs.

See the [main README](https://github.com/eyeco/sqid/blob/main/README.md) for more details about how to load plugins in sqıd, in particular the ` Modules` and `Run` sections in `Projects > sqıd`. 

_TODO: more detailed how-to-use_

## Disclaimer

**Note that this plugin is a minimal viable implementation.** Only fundamental features are available, however, the code will be extended on as and when required. Unfortunately, as currently there is no funding to do any of those, the further development is at the moment halted. When stepping through the source code, you will find passages that are marked as `TODO`. For hints on potential issues, keep an eye on the output printed to the command line window.

Bottom line: **use at your own discretion.** You may want to contact the author if questions arise, who will do his best to respond, depending on his momentary schedule.

### Contributing

Due to mentioned time constraints, there are aspects in both code and project configuration that are far from optimal but weren't yet addressed. In that light, collaborators are welcome; if you find the project interesting and useful and want to help improve it, don't hesitated to get in touch with the author.

## Crediting

The purpose, use cases (including intended architecture thereof), and (to some degree) usage of sqıd is elaborated in an [IEEE Pervasive Computing magazine](https://www.computer.org/csdl/magazine/pc) article, entitled [sqıd: A Multipurpose Virtualization Tool, Streamlining Setups for UbiComp Research](https://doi.org/10.1109/MPRV.2025.3627401) (2025) by Aigner et al. If you use the tool for your own research, you can show gratitude by citing the article in your own publication(s).

## Build

Dependencies:
- sqidCore [[link](https://github.com/eyeco/sqid)]
- TUIO 2.0 [[link](https://github.com/mkalten/TUIO20_CPP)]

## List of operators

- networking
	- tuio
		- tuio
		- toSampleFrame

## TODO

- implement missing data types
- implement more reasonable data converters

## Author

Roland Aigner [[link](https://www.rolandaigner.com)]

## License

Copyright (C) 2025 eyeco

sqıd Visual Programming Environment is licensed under the GPL3 License. See LICENSE file in the package root for license information.
