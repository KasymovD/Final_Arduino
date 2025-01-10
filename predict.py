from tensorflow.keras.models import load_model
from tensorflow.keras.preprocessing import image
import numpy as np

model_path = "Stamp/1.h5"
model = load_model(model_path)

def predict_image(img_path, threshold=0.9):
    img = image.load_img(img_path, target_size=(150, 150))
    img_array = image.img_to_array(img)
    img_array = np.expand_dims(img_array, axis=0)
    img_array /= 255.0

    prediction = model.predict(img_array)
    return "REAL" if prediction[0][0] >= threshold else "FAKE"

if __name__ == "__main__":
    test_image_path = "test_image.jpg"
    result = predict_image(test_image_path, threshold=0.9)
    print(f"{result}")
