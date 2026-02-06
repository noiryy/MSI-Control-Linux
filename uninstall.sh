#!/usr/bin/env bash
set -euo pipefail

PREFIX="/usr/local"

echo "Removing MSI Control GUI from ${PREFIX}..."
sudo rm -f "${PREFIX}/bin/msi-ctl-gui"
sudo rm -f "${PREFIX}/share/applications/msi-ctl.desktop"
sudo rm -f "${PREFIX}/share/icons/hicolor/scalable/apps/msi-ctl.svg"

if command -v update-desktop-database >/dev/null 2>&1; then
    sudo update-desktop-database "${PREFIX}/share/applications"
fi

echo "Uninstall complete."
