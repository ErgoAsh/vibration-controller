import yaml

# Global variable to store config
config = None


def load_config(config_file: str = "config.yaml"):
    global config
    with open(config_file, "r") as f:
        config = yaml.safe_load(f)
        print("Config loaded")


def save_config(config_file: str = "config.yaml", new_config=None):
    global config
    if new_config is not None:
        config = new_config

    if config is None:
        print("?? Config is null ??")
        return

    with open(config_file, "w") as f:
        config = yaml.dump(config, f, default_flow_style=False)
        print("Config saved")


load_config()
