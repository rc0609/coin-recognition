import os
from kaggle.api.kaggle_api_extended import KaggleApi

# Initialize the API
api = KaggleApi()
api.authenticate()

# Dataset name
dataset_name = "balabaskar/count-coins-image-dataset"
download_path = "datasets/coins"

# Download the dataset
os.makedirs(download_path, exist_ok=True)
api.dataset_download_files(dataset_name, path=download_path, unzip=True)

print(f"Dataset downloaded and extracted to: {download_path}")
