import cv2

def capture_image(output_path="test_image.jpg"):

    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        return None

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        cv2.imshow("Webcam", frame)

        if cv2.waitKey(1) & 0xFF == ord('s'):
            cv2.imwrite(output_path, frame)
            print(f"{output_path}")
            break

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
    return output_path

if __name__ == "__main__":
    capture_image()
