from camera import capture_image
from predict import predict_image
from arduino import send_to_arduino

def main():
    image_path = capture_image(output_path="test_image.jpg")
    if not image_path:
        return

    result = predict_image(image_path, threshold=0.9)
    print(f"{result}")

    send_to_arduino(result)

if __name__ == "__main__":
    main()

