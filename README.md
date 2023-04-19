= AppImage
https://appimage-builder.readthedocs.io/en/latest/intro/tutorial.html

run appimage-builder

= Windows builds

- Update version in https://invent.kde.org/packaging/craft-blueprints-kde/-/blob/master/kde/unreleased/isoimagewriter/isoimagewriter.py
- On a Windows computer open PowerShell and run C:\CraftRoot\craft\craftenv.ps1
- Run   `craft -i isoimagewriter`
- Run   `craft package --target=0.9.2 isoimagewriter`
