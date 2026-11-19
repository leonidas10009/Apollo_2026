# GameStream Migration
Nvidia announced that their GameStream service for Nvidia Games clients will be discontinued in February 2023.
Luckily, Apollo performance is now equal to or better than Nvidia GameStream.

> [!NOTE]
> This guide is inherited from Sunshine. GSMS is an upstream LizardByte tool; it produces a
> compatible `apps.json` that Apollo can import.

## Migration
We have developed a simple migration tool to help you migrate your GameStream games and apps to Apollo automatically.
Please check out the [GSMS](https://github.com/LizardByte/GSMS) project if you're interested in an automated
migration option. GSMS offers the ability to migrate your custom and auto-detected games and apps. The
working directory, command, and image are all set in Apollo's `apps.json` file. The box-art image is also copied
to a specified directory.

## Internet Streaming
If you are using the Moonlight Internet Hosting Tool, you can remove it from your system when you migrate to Apollo.
To stream over the Internet with Apollo and a UPnP-capable router, enable the UPnP option in the Apollo Web UI.

> [!NOTE]
> Running Apollo together with versions of the Moonlight Internet Hosting Tool prior to v5.6 will cause UPnP
> port forwarding to become unreliable. Either uninstall the tool entirely or update it to v5.6 or later.

## Limitations
Apollo does have some limitations, as compared to Nvidia GameStream.

* Automatic game/application list.
* Changing game settings automatically to optimize streaming.
