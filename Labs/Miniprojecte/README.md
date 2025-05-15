# Ubuntu Real Time
> Referencia principal: https://documentation.ubuntu.com/real-time/en/latest/

## [Install steps](https://documentation.ubuntu.com/pro-client/en/latest/howtoguides/enable_realtime_kernel/)

- Actualitzar sistema: `sudo apt update && sudo apt install ubuntu-advantage-tools`
- Activar real time kernel: `sudo pro enable realtime-kernel`
- Increase storage: (host) `multipass set local.str.disk=16G`