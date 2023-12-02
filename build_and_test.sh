pwd;
sudo python3 python/setup.py install;
# clear;
python3 python/pytest.py;
rm -dr build;