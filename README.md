# UltimatePhobia


## Downloading the base game

To download the correct version of the base game, download [DepotDownloader](https://github.com/SteamRE/DepotDownloader/releases) and run `DepotDownloader -app 739630 -depot 739631 -manifest 1539425284896419288 -username steamuser123`. Replace `steamuser123` with your Steam username.

## Patching CVE-2025-59489

```
wget -O UnityPlayer.dll https://security-patches.unity.com/bc0977e0-21a9-4f6e-9414-4f44b242110a/winplayers/b0280d3f53ca708c3ff088d253b2fa775e679225/UnityPlayerP.dll
```