# rsbbmp
Stand-alone RSB&lt;->BMP converter in C.

Back around 1999-2000, Red Storm Entertainment produced a series of games including Rainbow 6 & Rogue Spear which included the concept of an "arm patch" visible on players within the game.  As a multiplayer game, being able to create custom arm patches within the game was a lot of fun, and served as a branding technique for teams.

Redstorm released an Adobe Photoshop Plug-in to convert their RSB graphic format to something editable, but it required owning Photoshop (thereby preventing most players from creating arm patches).  I reverse engineered the format and created the RSB<->BMP conversion tool to allow anyone to create arm patches with any graphics editor they wanted to.

This was my 2nd C-based project and ended up being downloaded well over 100k times.

This code was significantly faster than the Adobe Photoshop Plugin Redstorm released.  It could convert hundreds of RSB files in just 1 or 2 seconds.  It's written in C utilizing the Win32API for a GUI with no other dependencies, so after applying upx, the final download executable size was only 10kb.

This repo contains the core algorithms extracted from RSB<->BMP for converting RSB to BMP, and BMP to RSB.  It does not include the original Win32 API front end code which remains closed-source.

While the original site no longer exists, this utility can still be found several places online for download including here: http://www.oocities.org/usr_rsclan/rsbbmp.zip
