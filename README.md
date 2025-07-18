# OpenVnmrJ_R - Support for Resynant spectrometer

This repository is used in conjunction with the [**OpenVnmrJ**](https://github.com/OpenVnmrJ/OpenVnmrJ) and [**ovjTools**](https://github.com/OpenVnmrJ/ovjTools) repositories to build a package to support the Resynant spectrometer.

This package must be built on an Ubuntu system. Network access will be required. The following instructions may be used.

1. If git does not exist on the computer, install it.
```
    sudo apt install git
```

2. Create a directroy to hold all build-related files
```
    cd ~
    mkdir ovjbuild
    cd ovjbuild
```
3. Get the OpenVnmrJ_R repository.
```
    git clone https://github.com/DanIverson/OpenVnmrJ_R.git
```

4. Install Ubuntu packages required for the build process.
```
    cd OpenVnmrJ_R/bin
    ./toolChain
```

5. Build the package. As a first step, the OpenVnmrJ and ovjTools repositories will automatically be downloaded. The build processes takes about 10 minutes.
```
    ./buildR
```
6. Success of the build process can be checked with the ```whatsin``` script.
```
    ./whatsin
```

7. Optional step to package OpenVnmrJ for distribution. The actual name of the zip file is selectable.
```
    cd ~/ovjbuild
    zip -ry OpenVnmrJ_R.zip dvdimageR
```

8. Install OpenVnmrJ
```
    cd ~/ovjbuild/dvdimageR
    ./load.nmr
```

9. Run OpenVnmrJ. You will probably need to logout and then login again. You can double-click the OpenVnmrJ Desktop icon and select "Trust and Launch". Alternatively, you can open a terminal window and enter
```
    vnmrj
```
10. After OpenVnmrJ starts, to configure the spectrometer, in the OpenVnmrJ command line, enter
```
    config
````

If you have built and installed OpenVnmrJ using the above procedure, you can upgrade it with the following steps.

1. Fetch the latest changes into the OpenVnmrJ_R repository
```
    cd ~/ovjbuild/OpenVnmrJ_R
    git pull
```

2. Build the package. The OpenVnmrJ and ovjTools repositories will automatically be updated with any changes. The build processes takes about 10 minutes.
```
    cd ~/ovjbuild/OpenVnmrJ_R/bin
    ./buildR
```

3. Upgrade OpenVnmrJ. In addition to the load.nmr script, there will be an upgrade.nmr script. You may have to exit any running OpenVnmrJ sessions and stop the acquisition communication programs (acqcomm stop)
```
    cd ~/ovjbuild/dvdimageR
    ./upgrade.nmr
```

