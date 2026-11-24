## Create service file
sudo nano /etc/systemd/system/cs4250startup.service


## Config
[Unit]
Description=Run projectthing run script
After=network.target

[Service]
Type=oneshot
User=root
ExecStart=/home/debian/cs4250startup.sh
RemainAfterExit=true

[Install]
WantedBy=multi-user.target


## Enable
sudo systemctl daemon-reload
sudo systemctl enable cs4250startup.service
sudo systemctl start cs4250startup.service


## Status Check
sudo systemctl status cs4250startup.service