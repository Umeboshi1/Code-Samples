using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerController : MonoBehaviour
{
    private Rigidbody2D rb;

    //Character movement
    public float moveSpeed;
    public float jumpSpeed;
    public float gravity;

    //For checking if character can jump or not
    public Transform groundCheckLeft;
    public Transform groundCheckRight;
    const float groundCheckRadius = 0.1f;
    [SerializeField] LayerMask GroundLayer;

    private bool leftButtonPressed;
    private bool rightButtonPressed;

    //Keeps track of what direction character is facing
    public bool isFacingRight;

    public bool isClimbing;
    private bool hasScroll;

    //Used for fliping the Character horizontally
    private float xScale;

    //Disable controls 
    private bool disableMovement;

    public Animator animator;

    public SceneManagement sceneManager;

    private Vector3 climbPos;
    private float direction = 1.0f;

    public bool grounded;

    // Start is called before the first frame update
    void Start()
    {
        rb = GetComponent<Rigidbody2D>();
        rb.gravityScale = gravity;
        //Initialize bools
        isFacingRight = true;
        disableMovement = false;
        xScale = transform.localScale.x;
        isClimbing = false;
        hasScroll = false;
        //animator = GetComponent<Animator>();
    }

    // Update is called once per frame
    void Update()
    {
        if (isClimbing)
        {
            rb.velocity = Vector3.zero;
            if (!isFacingRight)
            {
                direction = -1.0f;
            } else
            {
                direction = 1.0f;
            }
            rb.transform.position = Vector3.Lerp(
                new Vector3(rb.transform.position.x, rb.transform.position.y, rb.transform.position.z),
                new Vector3(climbPos.x + (0.0f * direction), climbPos.y - 4.5f, rb.transform.position.z), .3f);
        }
        if (!disableMovement)
        {
            if(leftButtonPressed && !rightButtonPressed) //Go left
            {
                rb.velocity = new Vector2(-moveSpeed, rb.velocity.y);
                if (isFacingRight)
                {
                    isFacingRight = false;
                    transform.localScale = new Vector2(-xScale, transform.localScale.y);
                }
            } else if(rightButtonPressed && !leftButtonPressed) //Go right
            {
                rb.velocity = new Vector2(moveSpeed, rb.velocity.y);
                if (!isFacingRight)
                {
                    isFacingRight = true;
                    transform.localScale = new Vector2(xScale, transform.localScale.y);
                }
            } else if(!leftButtonPressed && !rightButtonPressed)
            {
                rb.velocity = new Vector2(Mathf.Lerp(rb.velocity.x, 0, .3f), rb.velocity.y);
            }
            animator.SetFloat("moveSpeed", Mathf.Abs(rb.velocity.x));
        }
        if (hasScroll && grounded)
        {
            goToScroll();
        }
    }

    private void FixedUpdate()
    {
        grounded = (Physics2D.OverlapCircle(groundCheckLeft.position, groundCheckRadius, GroundLayer) ||
            (Physics2D.OverlapCircle(groundCheckRight.position, groundCheckRadius, GroundLayer)));
    }

    //initializes jump animation
    public void startJump()
    {
        if (!disableMovement && grounded)
        {
            disableMovement = true;
            //start animation
            animator.SetTrigger("jump");
        }
    }

    //allows movement input and starts jump
    public void jump()
    {
        disableMovement = false;
        rb.velocity = new Vector2(rb.velocity.x, jumpSpeed);
    }

    public void LeftButtonPressed()
    {
        leftButtonPressed = true;
    }

    public void RightButtonPressed()
    {
        rightButtonPressed = true;
    }

    public void LeftButtonReleased()
    {
        leftButtonPressed = false;
    }

    public void RightButtonReleased()
    {
        rightButtonPressed = false;
    }

    //locks player movement and starts pull up animation
    public void OnClimb(Vector3 pullUpPos)
    {
        isClimbing = true;
        disableMovement = true;
        animator.SetTrigger("pullUp");
        climbPos = pullUpPos;
        rb.gravityScale = 0.0f;
        rb.velocity = Vector3.zero;
    }

    //gives control back to player after climbing animation is complete
    public void endClimb()
    {
        isClimbing = false;
        disableMovement = false;
        rb.transform.position = new Vector3(climbPos.x + (direction * 0.35f), climbPos.y + 0.1f, rb.transform.position.z);
        rb.gravityScale = gravity;
    }

    //checks all collisions with trigger objects
    private void OnTriggerEnter2D(Collider2D collision)
    {
        if(collision.tag == "Scroll")
        {
            animator.SetTrigger("pickUp");
            disableMovement = true;
            GlobalData.Instance.currScrollIndex = collision.gameObject.GetComponent<Scroll>().scrollIndex;
            Destroy(collision.gameObject.GetComponent<Scroll>().scrollCheck);
            Destroy(collision.gameObject);
        }
    }

    public void startScroll()
    {
        disableMovement = false;
        hasScroll = true;
    }

    public void goToScroll()
    {
        hasScroll = false;
        sceneManager.goToScroll();
    }
}
