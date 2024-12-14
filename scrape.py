import os
import requests
from bs4 import BeautifulSoup

# Main URL
base_url = "https://www.usmint.gov"
main_url = "https://www.usmint.gov/news/image-library/circulating?utm_source=chatgpt.com"

# Directory to save images
output_dir = "coin_images"
os.makedirs(output_dir, exist_ok=True)

def download_image(image_url, save_path):
    """Download an image and save it to the specified path."""
    try:
        response = requests.get(image_url)
        response.raise_for_status()  # Ensure the request was successful
        with open(save_path, "wb") as file:
            file.write(response.content)
        print(f"Downloaded: {save_path}")
    except Exception as e:
        print(f"Failed to download {image_url}: {e}")

def scrape_images():
    """Scrape all coin images from the main link."""
    # Fetch the main page
    response = requests.get(main_url)
    soup = BeautifulSoup(response.content, "html.parser")

    # Find all category links
    category_links = soup.find_all("a", class_="LinkText")  # Adjust class as necessary
    for category_link in category_links:
        category_name = category_link.text.strip()
        category_url = base_url + category_link.get("href")

        # Create a folder for the category
        category_dir = os.path.join(output_dir, category_name)
        os.makedirs(category_dir, exist_ok=True)

        # Fetch the category page
        category_response = requests.get(category_url)
        category_soup = BeautifulSoup(category_response.content, "html.parser")

        # Find year links in the category page
        year_links = category_soup.find_all("a", class_="LinkText")  # Adjust class as necessary
        for year_link in year_links:
            year_name = year_link.text.strip()
            year_url = base_url + year_link.get("href")

            # Create a folder for the year
            year_dir = os.path.join(category_dir, year_name)
            os.makedirs(year_dir, exist_ok=True)

            # Fetch the year page
            year_response = requests.get(year_url)
            year_soup = BeautifulSoup(year_response.content, "html.parser")

            # Find all image links in the year page
            image_tags = year_soup.find_all("img")
            for img_tag in image_tags:
                img_url = img_tag.get("src")
                if img_url and "https" in img_url:  # Ensure it's a full URL
                    # Generate a save path for the image
                    img_name = os.path.basename(img_url)
                    save_path = os.path.join(year_dir, img_name)
                    # Download the image
                    download_image(img_url, save_path)

# Run the scraper
scrape_images()
