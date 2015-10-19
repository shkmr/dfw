;;;
;;;  Generate USB descriptors, and format them into C style array.
;;;
;;;                               written by skimu@mac.com
;;;

;;;    Following Non-R5RS functions are used in this file.
;;;
;;;    logand   : logical (bitwise) and
;;;    ash      : arithmetic shift
;;;    format   :
;;;    port-map :

;;;
;;;    TREE -> C style array
;;;
(define (make-c-array-write)
  (let ((count 0))
    (lambda (m)
      (define (write-byte x)
        (if (= count  0) (display "  "))
        (format #t "0x~2,'0x, " x)
        (set! count (+ count 1))
        (if (= count 8) (begin
                          (newline)
                          (set! count 0))))
      (define (set-count c)
        (set! count c))
      (case m
        ((set-c)  set-count)
        ((write)  write-byte)
        (else 
         (error "unknown method"))))))

(define caw                (make-c-array-write))
(define c-array-write      (caw 'write))
(define c-array-set-count  (caw 'set-c))

(define (tree-write tree)
  (let lp ((tree tree))
    (cond ((null? tree) #t)
          ((pair? tree)
           (tree-write (car tree))
           (lp (cdr tree)))
          (else
           (c-array-write tree)))))

(define (tree-length tree)
  (let lp ((len 0)
           (tree tree))
    (cond ((null? tree) len)
          ((pair? tree)
           (lp (+ len (tree-length (car tree)))
               (cdr tree)))
          (else 1))))

(define (tree-checksum tree)
  (let lp ((cs 0)
           (tree tree))
    (cond ((null? tree) (logand cs #xff))
          ((pair? tree)
           (lp (+ cs (tree-checksum (car tree)))
               (cdr tree)))
          (else tree))))

(define (integer->list len)
  (if (> len 65535)
      (error "integer should be within 16bit"))
  (list (logand len          #xff)
        (logand (ash len -8) #xff)))

;;;
;;;   USB descriptor
;;;
(define (descriptor descriptor-type . tree)
  (let ((len (+ 2 (tree-length tree))))
    (if (> len 255) (error "descritor too large"))
    (list len descriptor-type tree)))

(define (device-descriptor class 
                           subclass 
                           protocol 
                           ep0-packet-size
                           vendor-id
                           product-id
                           revision
                           index-of-manufacturer-string
                           index-of-product-name-string
                           index-of-serial-number-string
                           number-of-configurations)
  (descriptor #x01                      ; Device descriptor
              (integer->list #x0110)    ; USB 1.1
              class
              subclass
              protocol
              ep0-packet-size
              (integer->list vendor-id)
              (integer->list product-id)
              (integer->list revision)
              index-of-manufacturer-string
              index-of-product-name-string
              index-of-serial-number-string
              number-of-configurations))
              
(define (configuration-descriptor num-interfaces
                                  configuration-value
                                  index-of-string-descriptor-string
                                  attributes
                                  max-power-consumption
                                  interfaces)
  (let ((total-length (+ 9 (tree-length interfaces))))
    (list 9 
          #x02
          (integer->list total-length)
          num-interfaces
          configuration-value
          index-of-string-descriptor-string
          attributes
          max-power-consumption
          interfaces)))

(define (interface-descriptor interface-number 
                              alternate-setting
                              num-endpoints
                              class
                              subclass
                              protocol
                              index-ofstring-descriptor-string)
  (descriptor #x04 
              interface-number
              alternate-setting
              num-endpoints
              class
              subclass
              protocol
              index-ofstring-descriptor-string))

(define (endpoint-descriptor address
                             attributes
                             max-packet-size
                             interval)
  (descriptor #x05 address attributes 
              (integer->list max-packet-size)
              interval))

(define (string-descriptor str)
  (define (string->unicode-binary-tree str)
    (map (lambda (c) 
           (let ((b (char->integer c)))
             (if (> b  #x7f)
                 (error "illegal string charactor in string")
                 (list b 0))))
         (string->list str)))
  (let* ((uli (string->unicode-binary-tree str)))
    (descriptor #x03 uli)))

;;;
;;;    TUSB3410's boot EEPOM. (to be embeded in ../dfw/dfw)
;;;
(define (tusb-descriptor data-type tree)
  (let ((len       (tree-length    tree))
        (checksum  (tree-checksum  tree)))
    (list data-type
          (integer->list len) 
          (logand #xff checksum)
          tree)))
(define (tusb-device-descriptor tree)    (tusb-descriptor #x03 tree))
(define (tusb-string-descriptor tree)    (tusb-descriptor #x05 tree))

(define (tusb3410-boot-device-descriptor vendor-id product-id
                                         major-version-number minor-version-number
                                         manufacturer-string-index
                                         product-string-index
                                         serial-number-string-index)
  (descriptor #x01 (list #x10 #x01 
                         #xff #x00 #x00 #x08 
                         (integer->list vendor-id)
                         (integer->list product-id)
                         minor-version-number major-version-number
                         manufacturer-string-index
                         product-string-index
                         serial-number-string-index
                         #x01)))

(define (fetuif-descriptor-header vendor-id product-id)
  (define end-of-header 0)
  (list
   #x10 #x34
   (tusb-device-descriptor
    (tusb3410-boot-device-descriptor vendor-id product-id 1 1 1 2 3))
   (tusb-string-descriptor  (fetuif-string-descriptor))
   end-of-header))

(define (fetuif-autoexec-firmware vendor-id product-id firmware-name)
  (let* ((firm (with-input-from-file firmware-name
                 (lambda () (port-map (lambda (x) x) read-byte))))
         (size (length firm))
         (sum  (tree-checksum firm)))
    (define end-of-header 0)
    (list #x10 #x34 #x07 
          (integer->list size) 
          sum 
          firm
          end-of-header)))

;;;
;;;   USB descritpor for MSP-FET430UIF/DFW
;;;
(define (fetuif-device-descriptor vendor-id product-id revision)
  (device-descriptor #x02         ; Communication Class
                     0 0          ; SubClass and protocol
                     8            ; EP0 Packet size
                     vendor-id
                     product-id 
                     revision
                     1 2 3        ; indexes of string descriptors
                     1            ; Number of Configurations
                     ))

(define (fetuif-configuration-descriptor)
  (configuration-descriptor
   2                                 ; number of interfaces
   1                                 ; configuration value
   0                                 ; index of string descriptor for this configuratioin
   #x80                              ; attributes, bus powered
   50                                ; max-power 100mA
   (list                             ; interfaces
    (interface-descriptor 0          ; interface number
                          0          ; alternate setting
                          1          ; num-endpoints
                          #x02       ; class    : Communication Class Interface (CDC)
                          #x02       ; subclass : Abstract Control Model (ACM)
                          #x01       ; protocol : Common AT commands
                          0)         ; index of string descriptor string for this interface

    '(#x05 #x24 #x00 #x10 #x01)      ; CDC Header Functional Descriptor (CDC 1.1)
    '(#x05 #x24 #x01 #x00 #x01)      ; CDC Call Management Descriptor  (No call management, Data Class Interface: 1)
    '(#x04 #x24 #x02 #x02)           ; CDC ACM Functional Descriptor   (line_coding only)
    '(#x05 #x24 #x06 #x00 #x01)      ; CDC Union Functional Descriptor (MasterInterface: 0, SlaveInterface: 1)

    (endpoint-descriptor #x82
                         #x03        ; attribute : interrupt transfers
                         64          ; pack-packet-size
                         #xff)       ; interval
   
    (interface-descriptor 1          ; interface number
                          0          ; alternate setting
                          2          ; num-endpoints
                          #x0a       ; class    : Data Class Interface
                          #x00       ; subclass : No subclass for Data Class 
                          #x00       ; protocol : No class specific protocol required for Data Class
                          0)         ; index of string descriptor string for this interface

    (endpoint-descriptor #x01        ; address
                         #x02        ; attribute : bulk transfers
                         64          ; pack-packet-size
                         0)          ; interval (ignored for bulk endpoint)

    (endpoint-descriptor #x81        ; address
                         #x02        ; attribute : bulk transfers
                         64          ; pack-packet-size
                         0)          ; interval (ignored for bulk endpoint)
    )))

(define (fetuif-string-descriptor)
  ;; serial number string will be replaced by TUSB3410's device id, 
  ;; if the size of string is more than 16 bytes.
  (list
   (descriptor #x03 '(9 4))                 ; English
   (string-descriptor "Texas Instruments")  ; 1. manufacturer string
   (string-descriptor "MSP-FET430UIF/DFW")  ; 2. product string
   (string-descriptor "000")                ; 3. serial number string, (In MacOSX, this will create /dev/cu.usbmodem00x)
   #;(string-descriptor "0123456789abcdef") ; 3. serial number string, (Let firmware put TUSB3410's die ID)
   )) 

;;;
;;;
;;;
(define (gen fname func . args)
  (c-array-set-count 0)
  (with-output-to-file fname
    (lambda ()
      (let* ((tree (apply func args))
             (len  (tree-length tree)))
        (format #t "/*  generated by gen-descriptor-header.scm  */~%")
        (tree-write tree)
        (format #t "~%/*  ~d bytes  */~%" len)))))

(define (generate-inc-files vendor-id product-id)
  (gen "tusb3410_device_descriptor.inc" fetuif-device-descriptor vendor-id product-id #x0010)
  (gen "tusb3410_config_descriptor.inc" fetuif-configuration-descriptor)
  (gen "tusb3410_string_descriptor.inc" fetuif-string-descriptor)
  (gen "tusb3410_descriptor_header.inc" fetuif-descriptor-header vendor-id product-id))

(define (generate-autoexec  vendor-id product-id firmware-name)
  (gen "tusb3410_autoexec_firmware.inc" 
       fetuif-autoexec-firmware vendor-id product-id firmware-name))

(define (main args)
  (case (length args)

    ((4) (let ((vendor-id       (string->number (list-ref args 1) 16))
               (product-id      (string->number (list-ref args 2) 16))
               (firmware-name                   (list-ref args 3)))
           (generate-autoexec  vendor-id product-id firmware-name)))

    ((3) (let ((vendor-id       (string->number (list-ref args 1) 16))
               (product-id      (string->number (list-ref args 2) 16)))
           (generate-inc-files vendor-id product-id)))

    (else  (generate-inc-files #x0451    #xbeef)))
  0)
;;; EOF
