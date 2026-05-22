# ue-xelu-icons

[Xelu's FREE Controllers & Keyboard Prompts](https://thoseawesomeguys.com/prompts/) as an Unreal Plugin for programmatic use (cpp / bp)

<p align="center">
  <img width="460" height="312" src="https://user-images.githubusercontent.com/113832/152027390-cc88b427-7e8e-47be-a4ef-64cb673ba770.png">
</p>

This plugin exposes a Blueprint Library with helpers to return an icon texture (either as a soft object reference or resolved UTexture2D) matching either:

- a Key directly
- an Input name from Input Settings
- an Input Action and Mapping Context

Content folder also has a few UMG Blueprint widgets, but those are more for testing and demonstration purpose rather than being part of the core functionality of the plugin.

## Usage

Here is the list of BP Exposed methods:

![xelu_lib_1920](https://user-images.githubusercontent.com/113832/152030870-3e97c506-f387-4b7d-bbc0-19299f4d0a1e.png)

Those methods use internally a Datatable whose main purpose is to define the mapping between Unreal `FKey` and the path to the matching icon textures. Row name key (first column) follows the `IconType.Key` pattern where `IconType` can be one of the following value: `Keyboard`, `PS4`, `PS5`, `XboxOne`, `XboxSeries`.

Link to the google sheet: https://docs.google.com/spreadsheets/d/1QsfkbC6ATr4prhD_RdyfXtI1v61gKn3a10HULff1NYM

The icons datatable can be changed in Plugin Settings: `Project Settings > Plugins > Xelu Icons`

If you need to add or edit icon mappings, I recommend doing a duplicate of this Datatable (parent table row: `XeluIconsInputsMetaData`) and changing the configuration to match yours.

## Credits

The icon textures that can be found in the Content folder are coming from [Xelu's Controllers & Keyboard Prompts](https://thoseawesomeguys.com/prompts/).

Huge thanks to Nicolae Berbece and ["Those Awesome Guys"](https://thoseawesomeguys.com/) for releasing this pack in the public domain under Creative Commons 0 (CC0).

## License

Icon textures made by Nicolae remain under Creative Commons 0 (CC0).

All other code surrounding it related to making it available as an Unreal Engine plugin is MIT licensed.
