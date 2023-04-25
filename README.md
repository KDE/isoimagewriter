= Release process
- Release source with releaseme and put on download.kde.org
- Build Snap/flatpak/appimage/windows binaries and put on stores
- Add to isoimagewriter/org.kde.isoimagewriter.appdata.xml
- Announce!

= Snap Store
Builds and uploads fine
https://invent.kde.org/packaging/snapcraft-kde-applications/-/tree/Neon/release/isoimagewriter

= Flatpak
https://github.com/flathub/flathub/pull/4089

= AppImage
https://appimage-builder.readthedocs.io/en/latest/intro/tutorial.html run appimage-builder

Or builds on binary-factory but needs global theme icon made a local app icon.

= Windows builds

- Update version in https://invent.kde.org/packaging/craft-blueprints-kde/-/blob/master/kde/unreleased/isoimagewriter/isoimagewriter.py
- On a Windows computer open PowerShell and run C:\CraftRoot\craft\craftenv.ps1
- Run   `craft -i isoimagewriter`
- Run   `craft package --target=0.9.2 isoimagewriter`
- MS Store rejects it beacause it does not allow administrator for apps, there seems to be no equivalent of udisks2 so it can not be used there