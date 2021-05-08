.PHONY: clean All

All:
	@echo "----------Building project:[ QRC - Debug ]----------"
	@"$(MAKE)" -f  "QRC.mk"
clean:
	@echo "----------Cleaning project:[ QRC - Debug ]----------"
	@"$(MAKE)" -f  "QRC.mk" clean
